# LoRa ESP32-S3 Project

## Description
This project is an ESP32-S3-based device with LoRa (SX1278) support, powered by a lithium-ion battery (BL-4C) with a TP4056 charging module. The project is designed for wireless data transmission using LoRa technology.

## Components
- **Microcontroller:** [WeAct ESP32-S3-A DevKitC-1](https://mischianti.org/weact-esp32-s3-a-devkitc-1-high-resolution-pinout-datasheet-and-specs/?ysclid=m81uskhvyj967706428)
- **LoRa Module:** [SX1278](https://www.semtech.com/products/wireless-rf/lora-connect/sx1278)
- **Charging Module:** [TP4056](http://kungur.hldns.ru/pdf/TP4056.pdf)
- **Battery:** [Nokia BL-4C](https://www.wildberries.by/catalog/16442322/detail.aspx)

## Wiring
### Wiring LoRa SX1278 to ESP32 Basic
| SX1278 | ESP32 (GPIO) |
|--------|-------------|
| 3.3V   | 3.3V        |
| GND    | GND         |
| NSS    | GPIO15      |
| DIO0   | GPIO4       |
| SCK    | GPIO18      |
| MISO   | GPIO19      |
| MOSI   | GPIO23      |
| RST    | GPIO14      |

### Wiring LoRa SX1278 to ESP32-S3
| SX1278 | ESP32-S3 (GPIO) |
|--------|-----------------|
| 3.3V   | 3.3V            |
| GND    | GND             |
| NSS    | GPIO10          |
| DIO0   | GPIO9           |
| SCK    | GPIO12          |
| MISO   | GPIO13          |
| MOSI   | GPIO11          |
| RST    | GPIO14          |

## Firmware
To upload the firmware, use [this sketch](https://github.com/vpuhoff/lora-esp32/blob/master/lora.ino). The firmware enables sending and receiving data over a LoRa network using ESP32-S3.

## Device Assembly
1. Connect the LoRa SX1278 module to ESP32-S3 according to the wiring diagram.
2. Connect the lithium-ion BL-4C battery via the TP4056 module.
3. Connect the charging module to ESP32-S3 for battery-powered operation.
4. Ensure all connections are properly made.

## Usage
1. Upload the firmware to ESP32-S3.
2. Power on the device.
3. Start test data transmission via LoRa and verify its operation.

## Possible Improvements
- Add a display to show network status and device parameters.
- Optimize power consumption.
- Use a high-gain antenna to extend communication range.

## Contact
If you have any questions or suggestions, feel free to contact the project author via GitHub or other available channels.

