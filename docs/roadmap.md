# Roadmap

## 1. Establish a reproducible Wispier baseline
- [x] Import both ESP32 sketches with license and pinned provenance.
- [x] Enable BW16 defaults and Fahrenheit.
- [x] Fix credential edge whitespace, history comparison, and PEM termination.
- [ ] Compile both sketches successfully.
- [ ] Confirm PCB pin mapping, OLED, SD, GPS fix, B communication, BLE, and 5 GHz observations on Rob's hardware.
- [ ] Record a baseline scan/log rate.

## 2. Reliable WiGLE and WDGWars uploads
- Separate enable switches and credential fields; credential test and clear transport/auth/server errors.
- WiGLE API name and token encoded by the device; provide explicit migration for legacy encoded credentials.
- WDGWars multipart CSV upload: https://wdgwars.pl/api/upload-csv, field file, X-API-Key header.
- Validate WDGWars keys via /api/me. Reference: https://www.wdgwars.pl/press
- Check CSV compatibility using a real Wispier sample; do not assume all historical WiGLE header layouts are accepted.
- Stream closed files from SD with bounded RAM; validate TLS certificates and clock.
- SD-backed queue keyed by file content identity, with independent destination status.
- Pending, uploading, accepted/processing, complete, retryable failure, credential failure, and uncertain outcome states.
- Persist state with recovery after interrupted writes. Never delete local logs automatically.
- Reconcile interrupted requests where the server may have accepted the file before connection loss; do not promise exactly-once delivery without server support.
- Bounded exponential backoff with jitter, Retry-After handling, and manual retry.
- Default upload after session closure on saved Wi-Fi; measure scanning impact before allowing concurrent upload.
- UI must distinguish file acceptance from completed server processing.

Acceptance: test invalid keys, absent/stale CA, connection loss mid-upload, power loss during queue updates, duplicate scheduling, malformed/partial responses, 429, 5xx, SD-full, and one destination succeeding while the other fails.

## 3. Flock / Axon detection
- Audit maintained community lists and their redistribution licenses before importing.
- Versioned rules on SD, validated before activation; retain last known good version.
- Evidence types: MAC/OUI prefixes, Wi-Fi names, BLE names, manufacturer and service data.
- Match labels distinguish manufacturer-only evidence from specific device-family signatures.
- No fabricated OUI entries; record source URL, revision/date, rule type, and explanation.
- OLED alert and dashboard sightings with RSSI, first/last seen, GPS validity and detection reason.
- Preserve normal observation logging. Keep detection metadata in a separate log.
- Treat randomized addresses and generic hardware vendors explicitly; no implication of complete coverage.
- Test known positives and negatives from source fixtures; measure scan overhead.

## 4. Phone interface
Responsive status, upload queues, credential setup, session downloads, detector rules version and update status. No API secrets in logs, URLs, example configs, or dashboard responses.

