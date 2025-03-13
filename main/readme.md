# LoRa ESP32-S3 Project with Web Interface

## Description
This project is an ESP32-S3-based device with LoRa (SX1278) support, powered by a lithium-ion battery (BL-4C) with a TP4056 charging module. The project is designed for wireless data transmission using LoRa technology and includes a web-based management interface for easy configuration and monitoring.

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

## Firmware Features

### LoRa Communication
- LoRa packet transmission and reception with automatic acknowledgements
- Statistical tracking of packet delivery success rate
- Configurable LoRa parameters (SF, bandwidth, coding rate, TX power)
- RSSI monitoring

### Web Interface
- Real-time configuration via browser (no need to reflash)
- Dashboard with system status and delivery success graphs
- LoRa parameter configuration through user-friendly interface
- Comprehensive logging system
- Wi-Fi configuration (AP mode, STA mode, or both)

## Required Libraries
- [LoRa](https://github.com/sandeepmistry/arduino-LoRa) - LoRa module control
- [Adafruit_NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) - RGB LED control
- [WiFi](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi) - Wi-Fi management
- [LittleFS](https://github.com/lorol/LITTLEFS) - File system for ESP32
- [GyverDB](https://github.com/GyverLibs/GyverDB) - Database storage
- [GyverDBFile](https://github.com/GyverLibs/GyverDB) - File-based database
- [SettingsESPWS](https://github.com/GyverLibs/Settings) - Web-based UI configuration

## Installation and Usage
1. Install all required libraries through the Arduino Library Manager
2. Open and upload the project to your ESP32-S3
3. The device will create a Wi-Fi access point named "ESP32_LoRa" with password "12345678"
4. Connect to this network and navigate to http://192.168.4.1 in your browser
5. Use the web interface to monitor and configure the device

## Web Interface Sections
The web interface provides four main tabs:

### Dashboard
- System status overview
- Wi-Fi connection status
- LoRa packet statistics
- Real-time delivery success rate graph

### LoRa Status
- Current LoRa configuration
- Transmission statistics
- RSSI information

### Logs
- System event logs
- Adjustable log detail levels

### Settings
- Wi-Fi configuration
- LoRa parameter settings
- Device management options

## Device Operation
1. The device automatically transmits periodic test packets using the configured LoRa parameters
2. Each transmission is tracked and statistics are updated
3. All communications activity is logged and viewable in the web interface
4. Configuration changes can be made in real-time without restarting

## Power Optimization
The device is designed for battery operation with:
- Core tasks separation (LoRa on Core 1, Web interface on Core 0)
- Efficient task scheduling
- Web interface available only when needed

## Troubleshooting
- If the device fails to boot, check the serial output (115200 baud)
- LED/RGB indicators show device status:
  - Red blinks: Transmitting packet
  - Green blinks: Received packet
  - Blue blinks: Acknowledgement received

## Possible Improvements
- Power saving modes for extended battery life
- Data visualization improvements
- Additional sensors integration
- OTA firmware updates

## Contact
If you have any questions or suggestions, feel free to contact the project author via GitHub or other available channels.
