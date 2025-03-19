# LoRa ESP32/ESP32-S3 Project with Web Interface

## Description
This project implements a configurable LoRa communication system using either ESP32 or ESP32-S3 microcontrollers. It features a comprehensive web-based management interface for real-time monitoring, configuration, and logging. The system tracks packet delivery statistics, supports different WiFi operating modes, and provides visual feedback via LED indicators.

## Supported Hardware
- **Microcontrollers:** 
  - ESP32
  - ESP32-S3 [WeAct ESP32-S3-A DevKitC-1](https://mischianti.org/weact-esp32-s3-a-devkitc-1-high-resolution-pinout-datasheet-and-specs/?ysclid=m81uskhvyj967706428)
- **LoRa Module:** [SX1278](https://www.semtech.com/products/wireless-rf/lora-connect/sx1278)
- **Display:** ST7735S 1.44" 128x128 LCD (SPI interface)
- **Charging Module:** [TP4056](http://kungur.hldns.ru/pdf/TP4056.pdf)
- **Battery:** [Nokia BL-4C](https://www.wildberries.by/catalog/16442322/detail.aspx)
- **Case:** [3d printed](https://www.thingiverse.com/thing:6973923)
- **Power Options:** 
  - USB power
  - Battery power with TP4056 charging module (e.g., Nokia BL-4C)

## Wiring

### ESP32-S3 to LoRa SX1278
| SX1278 | ESP32-S3 (GPIO) |
|--------|-----------------|
| 3.3V   | 3.3V            |
| GND    | GND             |
| NSS    | GPIO10          |
| DIO0   | GPIO9           |
| SCK    | GPIO12 (default SPI)  |
| MISO   | GPIO13 (default SPI)  |
| MOSI   | GPIO11 (default SPI)  |
| RST    | GPIO14          |

### ESP32 to LoRa SX1278
| SX1278 | ESP32 (GPIO) |
|--------|-------------|
| 3.3V   | 3.3V        |
| GND    | GND         |
| NSS    | GPIO15      |
| DIO0   | GPIO4       |
| SCK    | GPIO18 (default SPI) |
| MISO   | GPIO19 (default SPI) |
| MOSI   | GPIO23 (default SPI) |
| RST    | GPIO14      |

### ESP32 to ST7735S LCD Display
| ST7735S | ESP32 (GPIO) |
|---------|-------------|
| VCC     | 3.3V        |
| GND     | GND         |
| SCL     | GPIO18 (shared SPI SCK) |
| SDA     | GPIO23 (shared SPI MOSI) |
| RES     | GPIO17 (TFT_RST) |
| DC      | GPIO16 (TFT_DC) |
| CS      | GPIO5 (TFT_CS) |
| BLK     | GPIO22 (TFT_LED) or 3.3V |

## Features

### Core Functionality
- Automatic dual-way LoRa packet transmission with acknowledgment system
- Dynamic statistics tracking (total packets, delivery success rate, RSSI)
- Multi-core task management (LoRa operations on Core 1, Web interface on Core 0)
- Configurable system parameters via web interface
- Visual feedback via LED/RGB indicators for system status
- LCD display for real-time information and status monitoring
- CPU and memory usage monitoring with detailed task statistics

### LoRa Configuration
- Adjustable Spreading Factor (SF7-SF12)
- Configurable Bandwidth (7.8kHz to 500kHz)
- Selectable Coding Rate (4/5 to 4/8)
- Transmission power control (2-20 dBm)
- Maximum transmission attempts setting

### Web Interface
- Six main sections: Dashboard, LoRa Status, Logs, Settings, Display, and System Monitor
- Real-time statistics with visual graph of packet delivery success
- Comprehensive system logs with adjustable detail levels
- WiFi configuration supporting three modes: AP, STA, or AP+STA
- Visual indicators for system status and packet delivery success
- Display configuration and control options
- System resource monitoring and task statistics

### LCD Display Interface
- Multiple information pages with automatic scrolling
- Welcome/logo page with version information
- LoRa status page showing current configuration and statistics
- WiFi connection status with signal strength indicator
- System information page with memory usage and uptime
- Recent log entries display
- CPU monitoring page with task statistics
- Status indicators (WiFi, LoRa, battery) in status bar
- Information and error message pop-ups

## Required Libraries
- [LoRa](https://github.com/sandeepmistry/arduino-LoRa) - LoRa module control
- [Adafruit_NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) - RGB LED control
- [WiFi](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi) - Wi-Fi management
- [LittleFS](https://github.com/lorol/LITTLEFS) - File system for ESP32
- [GyverDB](https://github.com/GyverLibs/GyverDB) - Database storage
- [GyverDBFile](https://github.com/GyverLibs/GyverDB) - File-based database
- [SettingsESPWS](https://github.com/GyverLibs/Settings) - Web-based UI configuration
- [GTimer](https://github.com/GyverLibs/GTimer) - For timing operations
- [Adafruit_GFX](https://github.com/adafruit/Adafruit-GFX-Library) - Graphics library
- [Adafruit_ST7735](https://github.com/adafruit/Adafruit-ST7735-Library) - ST7735 LCD driver

## Installation

1. Install all required libraries through the Arduino Library Manager
2. Open the project in the Arduino IDE
3. Select the appropriate board (ESP32 or ESP32-S3)
4. Compile and upload the code to your device

## Usage

### Initial Setup
1. The device creates a WiFi access point named "ESP32_LoRa" with password "12345678" by default
2. Connect to this network with your computer or smartphone
3. Open a web browser and navigate to http://192.168.4.1
4. Use the web interface to configure the device

### Web Interface Navigation

#### Dashboard Tab
- Shows system status (WiFi connection, uptime, free memory)
- Displays LoRa statistics (packets sent, delivered, success rate)
- Real-time graph of delivery success rate over time

#### LoRa Status Tab
- Current LoRa configuration parameters
- Transmission statistics (total packets, success rate)
- RSSI information

#### Logs Tab
- System event logs with timestamp
- Support for different log levels (info, warning, error, debug)
- Log clearing functionality

#### Settings Tab
- WiFi configuration (mode selection, credentials)
- LoRa parameters configuration
- Device restart option

#### Display Tab
- Toggle display on/off
- Adjust display brightness
- Set display timeout for power saving
- Enable/disable automatic page scrolling
- Set auto-scroll interval
- Buttons for manual page selection

#### System Monitor Tab
- Real-time CPU and memory usage statistics
- Detailed task list with:
  - Task state (running, blocked, ready)
  - Priority
  - Stack usage
  - CPU usage percentage
- Color-coded indicators for resource usage levels

### LCD Display Navigation
The LCD display cycles through several information pages:
1. **Logo page** - Displays project name and version
2. **LoRa Status** - Shows current LoRa parameters and statistics
3. **WiFi Status** - Shows current WiFi mode and connection details
4. **System Info** - Displays uptime, memory usage, and CPU load
5. **Logs** - Shows most recent system log entries

The display includes:
- Status bar at the top with WiFi and LoRa indicators
- Page indicator dots at the bottom
- Pages auto-scroll by default (configurable in settings)

## Operation Details

### LED Indicators
- ESP32-S3: RGB LED with color coding:
  - Red: Packet transmission
  - Green: Packet received
  - Blue: Acknowledgment received
- ESP32: Built-in LED blinks to indicate activity

### Communication Protocol
The system implements a simple packet exchange protocol:
1. Device sends "HLO:[packet_id]" messages at random intervals
2. Receiving device responds with "ACK:[packet_id]"
3. Statistics are updated based on successful acknowledgments

### System Architecture
- Multi-task design using FreeRTOS
- Mutex protection for LoRa module access
- Separate tasks for sending, receiving, web interface, and display updates
- Stack monitoring for system health
- CPU load monitoring with per-task statistics

## Troubleshooting

- If the device fails to boot or LoRa initialization fails, the LED will blink indefinitely
- Check serial output at 115200 baud for diagnostic information
- Web interface logs provide detailed system status and error information
- If WiFi connection is unstable, check the configured credentials in Settings
- LED/RGB indicators show device status:
  - Red blinks: Transmitting packet
  - Green blinks: Received packet
  - Blue blinks: Acknowledgement received
- If the display is not working:
  - Check its connections (especially CS, DC, and RST pins)
  - Verify display settings in the web interface
  - Check that the display is enabled in settings

## Customization Options

### Default Values (configurable in code)
- **LoRa Settings:**
  - Frequency: 433MHz
  - Spreading Factor: SF12
  - Bandwidth: 31.25kHz
  - Coding Rate: 4/8
  - TX Power: 10dBm
  - Max Attempts: 5

- **WiFi Settings:**
  - Default AP SSID: "ESP32_LoRa"
  - Default AP Password: "12345678"

- **Display Settings:**
  - Enabled by default
  - Brightness: 100%
  - Auto-scroll: Enabled
  - Scroll interval: 5 seconds
  - Timeout: 60 seconds

### Performance Optimization
- Adjust LoRa parameters based on your range and reliability requirements
- For longer battery life, increase transmission intervals
- For higher throughput, reduce Spreading Factor and increase Bandwidth
- Reduce display brightness or increase timeout to save power
- Monitor CPU usage in the System Monitor tab to identify resource-intensive tasks

## Future Enhancements
- Power saving modes for extended battery life
- OTA (Over-The-Air) firmware updates
- Additional sensor integration
- Data logging to SD card or cloud services
- Support for mesh networking with multiple LoRa nodes
- Enhanced display interfaces with touch support
- Graphical representation of RSSI and signal quality over time

## Debugging
- The system includes comprehensive logging
- Stack usage monitoring helps identify memory issues
- LED/RGB indicators provide visual feedback on system status
- CPU and task monitoring helps identify performance bottlenecks
- System Monitor tab provides detailed resource usage statistics

## License
Open source - feel free to modify and distribute with proper attribution.