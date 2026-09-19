# HackerBox 0115 target

Source: https://hackerboxes.com/products/hackerbox-0115-wispier

The hardware inventory is confirmed from the kit. The following GPIO assignments are traced from the imported firmware, **not independently checked against the HackerBox PCB schematic or continuity measurements**.

| Function | ESP32 | RX / input | TX / output | Notes |
| --- | --- | --- | --- | --- |
| Inter-board UART | A and B | 27 | 14 | HardwareSerial.begin takes RX before TX; upstream macro names are reversed relative to actual call arguments. Do not rewire based on macro labels. |
| GPS UART | A | 16 | 17 | Default 9600 baud |
| BW16 UART | B | 16 | 17 | BW16 mode uses 38400 baud |
| DS18B20 | B | 22 | 22 | OneWire |
| SD chip select | A | — | 5 | SPI clock 10 MHz; other pins use board defaults |
| OLED | A | — | — | 128×32 SSD1306, address 0x3C, Wire defaults |

Confirm generic ESP32 board default I2C/SPI pins and PCB routing before changing any wiring. There is no verified battery-voltage sensing circuit in this audit; battery percentage is not promised.

BW16 mode is enabled by default in this fork. B owns the BW16 link. BLE manufacturer/service-data detection will need changes on B and an extension to the A/B reporting protocol; an OUI match alone can use existing address observations.

The four-cell power shield does not establish battery telemetry availability.

