#pragma once

#include <Arduino.h>
#include <SD.h>
#include <cJSON.h>
#include <esp_http_client.h>
#include <esp_random.h>
#include <mbedtls/base64.h>

namespace session_upload {

enum Destination { WIGLE, WDGWARS, WARDRIFT };

static bool isSession(const String &path) {
  if (!path.startsWith("/wd3-") || !path.endsWith(".csv") || path.length() <= 9) return false;
  for (unsigned i = 5; i < path.length() - 4; i++) {
    if (path[i] < '0' || path[i] > '9') return false;
  }
  return true;
}

static String readCardValue(const char *path, size_t limit) {
  File file = SD.open(path, FILE_READ);
  if (!file || file.isDirectory() || !file.size() || file.size() > limit) return "";
  String value;
  if (!value.reserve(file.size())) return "";
  while (file.available()) {
    int next = file.read();
    if (next <= 0 || !value.concat(static_cast<char>(next))) return "";
  }
  value.trim();
  return value;
}

static bool headerValue(const String &value) {
  if (value.isEmpty()) return false;
  for (unsigned i = 0; i < value.length(); i++) {
    if (value[i] < '!' || value[i] > '~') return false;
  }
  return true;
}

static String wigleCredential(const String &legacy) {
  String name = readCardValue("/wigle_api_name.txt", 256);
  String token = readCardValue("/wigle_api_token.txt", 256);
  if (name.isEmpty() && token.isEmpty() &&
      !SD.exists("/wigle_api_name.txt") && !SD.exists("/wigle_api_token.txt")) return legacy;
  if (!headerValue(name) || !headerValue(token) || name.indexOf(':') >= 0) return "";
  String pair = name + ":" + token;
  unsigned char encoded[689];
  size_t length = 0;
  if (mbedtls_base64_encode(encoded, sizeof(encoded), &length,
                           reinterpret_cast<const unsigned char *>(pair.c_str()),
                           pair.length()) != 0) return "";
  return String(reinterpret_cast<const char *>(encoded));
}

static bool readLine(File &file, String &line) {
  line = "";
  while (file.available()) {
    int c = file.read();
    if (c == '\n') {
      if (line.endsWith("\r")) line.remove(line.length() - 1);
      return true;
    }
    if (c <= 0 || line.length() >= 1024 || !line.concat(static_cast<char>(c))) return false;
  }
  return !line.isEmpty();
}

static const char legacyColumns[] =
    "MAC,SSID,AuthMode,FirstSeen,Channel,RSSI,CurrentLatitude,CurrentLongitude,AltitudeMeters,AccuracyMeters,Type";
static const char uploadColumns[] =
    "MAC,SSID,AuthMode,FirstSeen,Channel,Frequency,RSSI,CurrentLatitude,CurrentLongitude,AltitudeMeters,AccuracyMeters,RCOIs,MfgrId,Type";

static bool csvNumber(const String &field, double low, double high) {
  char *end = nullptr;
  const char *start = field.c_str();
  double value = strtod(start, &end);
  return end != start && *end == '\0' && value >= low && value <= high;
}

static unsigned wifiFrequency(int channel) {
  if (channel >= 1 && channel <= 13) return 2407 + channel * 5;
  if (channel == 14) return 2484;
  if (channel >= 32 && channel <= 177) return 5000 + channel * 5;
  return 0;
}

static String convertRow(const String &line) {
  // Native 1.4 rows have fixed fields after the SSID. Split from the right
  // so that even an older log with a comma in its name keeps its columns.
  String fields[11];
  int end = line.length();
  for (int i = 10; i >= 2; i--) {
    if (end <= 0) return "";
    int comma = line.lastIndexOf(',', end - 1);
    if (comma < 0) return "";
    fields[i] = line.substring(comma + 1, end);
    end = comma;
  }
  if (line.indexOf(',') != 17 || end <= 17) return "";
  if (fields[10] != "WIFI" && fields[10] != "BLE") return "";
  if (!csvNumber(fields[6], -90, 90) || !csvNumber(fields[7], -180, 180) ||
      !csvNumber(fields[5], -127, 0) || fields[3].length() != 19 ||
      fields[3].compareTo("2020-01-01 00:00:00") < 0) return "";
  String name = line.substring(18, end);
  name.replace("\"", "\"\"");
  name.replace("\r", " ");
  unsigned frequency = fields[10] == "WIFI" ? wifiFrequency(fields[4].toInt()) : 0;
  String row = line.substring(0, 17) + ",\"" + name + "\"";
  for (int i = 2; i <= 9; i++) {
    row += "," + fields[i];
    if (i == 4) {
      row += ",";
      if (frequency) row += String(frequency);
    }
  }
  return row + ",,0," + fields[10] + "\n";
}

static const char stagingPath[] = "/.session-upload.tmp";

static String stage(File &source, size_t limit) {
  String line;
  if (!readLine(source, line) || !line.startsWith("WigleWifi-1.4,")) return "Unsupported session format";
  String metadata = "WigleWifi-1.6" + line.substring(13) + "\n" + uploadColumns + "\n";
  if (!readLine(source, line) || line != legacyColumns) return "Unsupported CSV columns";
  if (SD.exists(stagingPath) && !SD.remove(stagingPath)) return "Could not clear staging file";
  File converted = SD.open(stagingPath, FILE_WRITE);
  if (!converted || converted.print(metadata) != metadata.length()) return "SD staging write failed";
  size_t length = metadata.length();
  size_t rows = 0;
  while (source.available()) {
    if (!readLine(source, line)) return "Session has an unreadable row";
    String row = convertRow(line);
    if (row.isEmpty()) continue;
    length += row.length();
    if (length > limit) return "Session exceeds destination upload limit";
    if (converted.print(row) != row.length()) return "SD staging write failed";
    rows++;
  }
  converted.flush();
  converted.close();
  File staged = SD.open(stagingPath, FILE_READ);
  if (!staged || staged.size() != length) return "SD staging size mismatch";
  return rows ? "" : "No geolocated Wi-Fi or BLE rows";
}

static bool sendBytes(esp_http_client_handle_t http, const char *data, size_t len) {
  while (len) {
    int written = esp_http_client_write(http, data, len);
    if (written <= 0) return false;
    data += written;
    len -= written;
  }
  return true;
}

static String receipt(esp_http_client_handle_t http, Destination service) {
  int64_t length = esp_http_client_fetch_headers(http);
  if (length < 0) return "Upload receipt missing; check account before retrying";
  int status = esp_http_client_get_status_code(http);
  if (status == 401 || status == 403) return "API credential rejected (HTTP " + String(status) + ")";
  if (status == 429) return "Rate limited (HTTP 429)";
  if (status != 200) return "HTTP " + String(status) + "; check account before retrying";

  char body[2048];
  size_t used = 0;
  while (used < sizeof(body) - 1 && !esp_http_client_is_complete_data_received(http)) {
    int n = esp_http_client_read(http, body + used, sizeof(body) - 1 - used);
    if (n <= 0) break;
    used += n;
  }
  if (!esp_http_client_is_complete_data_received(http)) return "Incomplete receipt; check account before retrying";
  body[used] = '\0';
  cJSON *json = cJSON_ParseWithOpts(body, nullptr, true);
  const char *key = service == WIGLE ? "success" : (service == WDGWARS ? "ok" : "accepted");
  bool accepted = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(json, key));
  bool duplicate = service == WARDRIFT && cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(json, "duplicate"));
  cJSON_Delete(json);
  if (duplicate) return "Already received by WarDrift";
  if (!accepted) return "Acceptance unconfirmed; check account before retrying";
  return service == WIGLE ? "Accepted by WiGLE for processing" : "Accepted";
}

static String transfer(File &file, Destination service, const String &name,
                       const String &credential, const String &pem, bool donate) {
  const char *urls[] = {"https://api.wigle.net/api/v2/file/upload",
                        "https://wdgwars.pl/api/upload-csv",
                        "https://wardrift.net/v1/ingest/signals"};
  esp_http_client_config_t config = {};
  config.url = urls[service];
  config.method = HTTP_METHOD_POST;
  config.cert_pem = pem.c_str();
  config.timeout_ms = 8000;
  config.disable_auto_redirect = true;
  esp_http_client_handle_t http = esp_http_client_init(&config);
  if (!http) return "Could not allocate HTTPS client";

  String boundary = "wispier-" + String(esp_random(), HEX) + String(esp_random(), HEX);
  String prefix, suffix;
  String contentType = "text/csv";
  if (service != WARDRIFT) {
    contentType = "multipart/form-data; boundary=" + boundary;
    prefix = "--" + boundary + "\r\nContent-Disposition: form-data; name=\"file\"; filename=\"" +
             name + "\"\r\nContent-Type: text/csv\r\n\r\n";
    suffix = "\r\n";
    if (service == WIGLE && donate) {
      suffix += "--" + boundary + "\r\nContent-Disposition: form-data; name=\"donate\"\r\n\r\non\r\n";
    }
    suffix += "--" + boundary + "--\r\n";
  }
  String authorization = service == WIGLE ? "Basic " + credential : credential;
  esp_http_client_set_header(http, "Content-Type", contentType.c_str());
  esp_http_client_set_header(http, "Accept", "application/json");
  esp_http_client_set_header(http, service == WIGLE ? "Authorization" : "X-API-Key", authorization.c_str());
  if (service == WARDRIFT) esp_http_client_set_header(http, "X-File-Name", name.c_str());

  size_t remaining = file.size();
  String result = "HTTPS connection failed; check Wi-Fi, clock and CA file";
  if (esp_http_client_open(http, prefix.length() + remaining + suffix.length()) == ESP_OK) {
    bool sent = sendBytes(http, prefix.c_str(), prefix.length());
    uint8_t buffer[1024];
    const unsigned long started = millis();
    while (sent && remaining && millis() - started < 120000UL) {
      size_t request = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
      int n = file.read(buffer, request);
      sent = n > 0 && sendBytes(http, reinterpret_cast<const char *>(buffer), n);
      if (sent) remaining -= n;
    }
    sent = sent && remaining == 0 && sendBytes(http, suffix.c_str(), suffix.length());
    result = sent ? receipt(http, service) : "Transfer interrupted; check account before retrying";
  }
  esp_http_client_cleanup(http);
  return result;
}

static String upload(const String &path, Destination service, const String &name,
                     const String &wigleKey, bool donate) {
  if (!isSession(path)) return "Invalid Wispier session";
  String safeName = name;
  for (unsigned i = 0; i < safeName.length(); i++) {
    char c = safeName[i];
    if (!isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '.' && c != '-') safeName.setCharAt(i, '_');
  }
  const char *caFiles[] = {"/wigle.crt", "/wdgwars.crt", "/wardrift.crt"};
  String credential = service == WIGLE ? wigleCredential(wigleKey) :
      readCardValue(service == WDGWARS ? "/wdgwars_key.txt" : "/wardrift_key.txt", 512);
  if (!headerValue(credential)) return "Missing or invalid API credential";
  String pem = readCardValue(caFiles[service], 8192);
  if (!pem.startsWith("-----BEGIN CERTIFICATE-----")) return "Missing or invalid CA file " + String(caFiles[service]);
  File source = SD.open(path, FILE_READ);
  if (!source || source.isDirectory()) return "Could not open saved session";
  if (service == WIGLE) {
    String line;
    if (!readLine(source, line) || !line.startsWith("WigleWifi-1.") ||
        !readLine(source, line) || !source.available() || !source.seek(0)) return "No observations in session";
    if (source.size() > 40U * 1024 * 1024) return "Session exceeds firmware upload limit";
    return transfer(source, service, safeName, credential, pem, donate);
  }
  size_t limit = (service == WARDRIFT ? 10U : 38U) * 1024 * 1024;
  String result = stage(source, limit);
  source.close();
  if (result.isEmpty()) {
    File staged = SD.open(stagingPath, FILE_READ);
    result = staged ? transfer(staged, service, safeName, credential, pem, false) : "Could not read staged CSV";
    staged.close();
  }
  SD.remove(stagingPath);
  return result;
}

}  // namespace session_upload
