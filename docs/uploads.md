# Upload saved Wispier sessions

Side A serves the upload page during the existing saved Wi-Fi window at boot. Use the local IP shown on the OLED. Open **Upload session** beside a saved drive and choose WiGLE, WDGWars, or WarDrift. Repeat for each destination you want to receive the session. The response page and OLED show the submit result. Use a trusted Wi-Fi network for the local dashboard.

## SD card setup

Put the credential and certificate files for the services you use in the root of the Wispier SD card. Each credential text file contains one value. Use API credentials from your account.

| Destination | Credential files | PEM trust anchor file | HTTPS host |
| --- | --- | --- | --- |
| WiGLE | wigle_api_name.txt and wigle_api_token.txt | wigle.crt | api.wigle.net |
| WDGWars | wdgwars_key.txt | wdgwars.crt | wdgwars.pl |
| WarDrift | wardrift_key.txt | wardrift.crt | wardrift.net |

Use the trusted public CA root for the HTTPS host’s current certificate chain. Save each trust anchor in PEM format, with a BEGIN CERTIFICATE line. An existing valid wigle.crt can stay on the card. These files are read from SD, so CA changes can be applied without reflashing. The device needs a valid clock for HTTPS. Startup attempts NTP on the configured network.

When both WiGLE text files are absent, the previously saved **Encoded for use** credential still works. Adding the two files makes the firmware build Basic authentication from the API name and token. If the OLED or web page reports HTTP 401 or 403, check the credential for the selected service. Keep credential files with the device and out of version control.

## What each service receives

- WiGLE receives the original capture as multipart CSV. Its acceptance message means it queued the file for processing. The existing WiGLE automatic-upload preference still applies.
- WDGWars receives a WiGLE 1.6 multipart CSV. WarDrift receives WiGLE 1.6 as a raw CSV request body. For both, the firmware stages Wi-Fi and BLE observations with coordinates and timestamps from the original 1.4 log. The conversion fills Wi-Fi frequencies for 2.4 GHz and 5 GHz channels. BLE frequency stays blank because Wispier does not log it. These destinations are manual.
- Original captures stay on the card. For WDGWars and WarDrift, the SD card needs free space for one staged copy. Converted uploads are limited to 38 MiB for WDGWars and 10 MiB for WarDrift. WiGLE uploads are capped at 40 MiB by the firmware.
- If the response is missing after a transfer, look for the session in the destination account before submitting it again. The firmware does not maintain a persistent receipt for manual WDGWars or WarDrift uploads.

## Firmware validation

Build Side A using the [pinned Arduino setup](build.md). For hardware bring-up, record a short real drive with a GPS fix, including a known 2.4 GHz and 5 GHz access point. Reboot into the saved Wi-Fi window, choose that session, and confirm the receipt and resulting observations in each account. Use the original SD capture to verify channels, coordinates and timestamps.

[WiGLE API](https://api.wigle.net/swagger) · [WDGWars developer API](https://www.wdgwars.pl/press) · [WarDrift device integration reference](https://github.com/justcallmekoko/ESP32Marauder/pull/1634)
