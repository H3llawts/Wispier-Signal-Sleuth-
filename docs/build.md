# Development build

This is an Arduino-ESP32 project. Do not use idf.py set-target esp32c5 for these ESP32-WROOM boards.

Pinned dependencies are in boards.txt and libraries.txt. Install Arduino CLI separately, then from the repository root:

```bash
arduino-cli core update-index --additional-urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.5 --additional-urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
arduino-cli lib install "MicroNMEA@2.0.6" "Adafruit GFX Library@1.11.10" "Adafruit SSD1306@2.5.11" "OneWire@2.3.8" "GParser@1.5.2" "NimBLE-Arduino@2.3.9"
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=min_spiffs,FlashMode=dio,FlashFreq=80,FlashSize=4M,DebugLevel=info --output-dir build/A A
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=min_spiffs,FlashMode=dio,FlashFreq=80,FlashSize=4M,DebugLevel=info --output-dir build/B B
```

The compile commands can also be run in PowerShell. Arduino IDE users can install these same versions and use ESP32 Dev Module with 4 MB flash, DIO, 80 MHz flash and Minimal SPIFFS (1.9 MB APP with OTA). Verify module flash size before flashing.

Build A and B separately. Never flash an ESP32 binary to the BW16. A normal app .bin is not automatically a merged image for offset zero.

The unmodified BW16 companion source is included in BW16-Open-AT/. It requires the separate Ameba toolchain. Preserve the working BW16 firmware while bringing up these changes.

Copy examples/cfg.txt to the root of a FAT32 SD card if needed. These settings are understood by the current sketches. No real credentials belong in this repository.

Compilation verifies toolchain compatibility and image fit only. Hardware verification remains required before a release.

