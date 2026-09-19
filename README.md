# Wispier Signal Sleuth

Updated Wispier/Signal Sleuth firmware for Rob's **HackerBox 0115 Wispier**.

**Development foundation, not a tested firmware release.** This repository now contains both ESP32 sketches based on Joseph Hewitt's wardriver_rev3. Dual-service uploads and Flock/Axon detection are planned; they are not implemented yet.

## Hardware
Two ESP32-WROOM-32U modules, BW16 RTL8720DN, ATGM336H GPS, microSD, 128×32 I2C OLED, and DS18B20. This target is the HackerBox PCB, not a generic ESP32-C5 or a different Signal Sleuth revision.

## First changes
- Enable BW16 mode by default on A and B.
- Default temperature display to Fahrenheit (cfg.txt can override).
- Trim WiGLE credential whitespace on save and load.
- Fix upload-history lookup to compare both file ID and file size.
- Zero-initialize WiGLE PEM buffers to preserve their terminating NUL.
- Disable upstream automatic OTA selection for this fork.

These changes do **not** establish the cause of the reported invalid-token error. The legacy uploader still expects WiGLE's full **Encoded for use** credential and uses a CA file on SD. Its authentication UI and transport need further work.

## Layout
- `A/A.ino`: GPS, SD logging, OLED, Wi-Fi scanning, web interface, legacy WiGLE uploader.
- `B/B.ino`: BLE/Wi-Fi scanning, temperature, BW16 communication.
- `boards.txt`, `libraries.txt`: upstream-pinned Arduino dependencies.
- [Hardware notes](docs/hardware.md)
- [Build instructions](docs/build.md)
- [Roadmap and acceptance criteria](docs/roadmap.md)
- [Upstream provenance and changes](docs/upstream.md)

## Build and status
Use Arduino, following [docs/build.md](docs/build.md). There are two separate ESP32 images. The BW16 uses its own firmware/toolchain; its companion source is included in `BW16-Open-AT/`.

A GitHub Actions workflow compiles both ESP32 sketches. A successful compilation is not a hardware validation. No release binaries have been approved for flashing.

## Credits and license
Based on [JosephHewitt/wardriver_rev3](https://github.com/JosephHewitt/wardriver_rev3), with its GPL-3.0 license and author notices retained. BW16 companion: [CoD-Segfault/BW16-Open-AT](https://github.com/CoD-Segfault/BW16-Open-AT). Hardware: [HackerBox 0115](https://hackerboxes.com/products/hackerbox-0115-wispier). Signal Sleuth inspiration: [463N7](https://github.com/463N7/SignalSleuth).

