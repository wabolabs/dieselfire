# DieselFire S3

A modernized open-source controller for generic Chinese diesel heaters, forked from [Afterburner](https://gitlab.com/mrjones.id.au/bluetoothheater) by Ray Jones.

## Overview

DieselFire S3 is a complete redesign of the Afterburner project, updated with modern hardware while preserving the core functionality that made Afterburner popular:

- **Diesel heater control** via the "blue wire" half-duplex serial protocol (25000 baud)
- **WiFi connectivity** for web interface, MQTT, and OTA updates
- **Bluetooth Low Energy** for mobile app control
- **Environmental monitoring** with temperature, humidity, pressure, and CO detection
- **Color touchscreen UI** with LVGL graphics
- **Programmable timers** (14 channels)
- **Fuel gauge** and hour meter
- **Smart error detection**

## Hardware

| Component | Specification |
|---|---|
| **MCU** | ESP32-S3-WROOM-1-N8R8 (8MB flash, 8MB PSRAM) |
| **Display** | 2.8" 320×240 ILI9341 TFT with capacitive touch (GT911) |
| **Ambient Sensor** | BME280 (temperature, humidity, pressure/altitude) |
| **CO Sensor** | MQ-7 carbon monoxide (external module) |
| **RTC** | DS3231 with CR2032 battery backup |
| **External Temp** | DS18B20 waterproof probe (optional) |
| **Power** | 12V DC input (heater system voltage) |
| **Connectivity** | USB-C (programming/debug), WiFi, BLE 5 |
| **Expansion** | 2× 2×10 GPIO headers |

## License

DieselFire S3 is released under the **GNU General Public License v3.0**.

This project is a **fork** of [Afterburner](https://gitlab.com/mrjones.id.au/bluetoothheater) by Ray Jones (Copyright 2018-2020). The original Afterburner code was used as the foundation and has been substantially modified and modernized. All original Afterburner code remains under GPLv3 as per the original license.

See [LICENSE](LICENSE) for the full license text.

## Quick Start

1. **Hardware**: Order PCB from JLCPCB/PCBWay using Gerber files in `gerber/`
2. **Firmware**: Install PlatformIO, then run `pio run` to build
3. **Flash**: Connect via USB-C and run `pio run --target upload`
4. **Configure**: Connect to the AP "DieselFire-S3" (password: thereisnospoon) or connect to existing WiFi

## Project Structure

```
DieselFire-S3/
├── boards/                  # PlatformIO board definitions
├── docs/
│   ├── design/              # Design documents and specifications
│   ├── firmware/            # Firmware documentation
│   └── hardware/            # Hardware documentation
├── gerber/                  # PCB fabrication files
├── kicad/
│   ├── schematic/           # KiCad schematic files
│   ├── pcb/                 # KiCad PCB layout files
│   ├── footprints/          # Custom KiCad footprints
│   └── libraries/           # KiCad symbol libraries
├── lib/                     # PlatformIO libraries (vendored)
├── src/
│   ├── cfg/                 # Configuration headers
│   ├── Display/             # LVGL display driver and screens
│   ├── Protocol/            # Blue wire protocol handling
│   ├── RTC/                 # Real-time clock and timers
│   ├── Utility/             # Utility classes (sensors, storage, etc.)
│   ├── WiFi/                # WiFi, web server, OTA
│   ├── Bluetooth/           # BLE implementation
│   └── OLED/                # Legacy OLED code (to be removed)
├── platformio.ini           # PlatformIO project configuration
├── README.md
└── LICENSE
```

## Features

### Heater Control
- Power on/off commands
- Temperature setpoint adjustment
- Fuel mixture tuning
- Pump priming
- Thermostat mode (cyclic, stop/start, linear)
- Frost protection mode
- Humidity-based control
- OEM controller co-existence mode

### Connectivity
- **WiFi**: STA mode (connect to existing network) or AP mode (captive portal)
- **BLE 5**: Bluetooth Low Energy for mobile app control
- **MQTT**: Publish heater status and receive commands
- **Web Server**: HTTPS web interface with WebSocket JSON streaming
- **OTA**: Over-the-air firmware updates

### Sensors
- **BME280**: Ambient temperature, humidity, pressure (altitude)
- **MQ-7**: Carbon monoxide detection (external module)
- **DS18B20**: External waterproof temperature probe (optional)
- **SCD41**: CO2 sensing (optional, not on default board)

### User Interface
- **2.8" Color Touchscreen**: 320×240 ILI9341 with capacitive touch
- **LVGL Graphics**: Hardware-accelerated UI with PSRAM buffer
- **Menu System**: Hierarchical menus for all settings
- **Timer Chart**: Visual overview of 14 programmable timers
- **Status Display**: Real-time heater status, sensor readings, alerts

### Safety
- **CO Detection**: MQ-7 sensor with alarm LED
- **Low Voltage Cutout**: Auto-shutdown when battery voltage is too low
- **Error Detection**: Smart monitoring of heater state transitions
- **Watchdog**: Hardware and software watchdog for reliability

## Pinout

### ESP32-S3 GPIO Mapping

| Peripheral | GPIO | Interface | Notes |
|---|---|---|---|
| **Native USB** | IO19 (DP) / IO20 (DN) | USB 1.1 | USB-C, programming + debug |
| **SPI Display** | IO18 (MOSI) / IO17 (SCK) / IO16 (MISO) | SPI (HSPI) | ILI9341, 320×240 |
| **Display CS** | IO15 | GPIO | TFT chip select |
| **Display DC** | IO14 | GPIO | TFT data/command |
| **Display RST** | IO13 | GPIO | TFT reset |
| **Display BL** | IO12 | GPIO/PWM | TFT backlight |
| **Touch I2C** | IO21 (SDA) / IO10 (SCL) | I2C (GPIO) | GT911 capacitive touch |
| **Touch INT** | IO9 | GPIO | Touch interrupt |
| **Touch RST** | IO8 | GPIO | Touch reset |
| **Blue Wire UART** | IO43 (TX) / IO44 (RX) | UART0 @ 25kbps | Heater communication |
| **Tx Gate** | IO47 | GPIO | 74LCX125 control |
| **BME280 I2C** | IO21 (SDA) / IO22 (SCL) | I2C (VSPI) | Temp/RH/Pressure |
| **DS3231 I2C** | IO21 (SDA) / IO22 (SCL) | I2C (VSPI) | RTC |
| **DS18B20** | IO45 | OneWire | External temp probe |
| **MQ-7 AOUT** | IO4 | ADC1 (12-bit) | CO analog reading |
| **MQ-7 DOUT** | IO5 | GPIO input | CO digital alert |
| **Status LED** | IO48 | GPIO | Green, active-low |
| **CO Alarm LED** | IO44 | GPIO | Red, active-low |
| **Power Button** | IO0 | GPIO (input) | With pull-up |
| **Reset Button** | IO5 | GPIO | Standard boot button |

### Expansion Headers

**H1 (2×10):** GPIO33, GPIO34, GPIO35, GPIO36, GPIO37, GPIO38, GPIO39, GPIO40, 3.3V, GND, GND
**H2 (2×10):** GPIO41, GPIO42, GPIO43, GPIO44, GPIO45, GPIO46, GPIO47, GPIO48, 3.3V, GND, GND

### Connectors

| Connector | Type | Purpose |
|---|---|---|
| J1 | USB-C | Programming, debug, power |
| J2 | 2-pin terminal block (3.81mm) | 12V power input |
| J3 | 3-pin JST-XH (2.5mm) | Blue wire to heater |
| J4 | 3-pin JST-XH (2.5mm) | DS18B20 external temp sensor |
| J5 | 4-pin JST-XH (2.5mm) | MQ-7 CO sensor |
| H1, H2 | 2×10 header (2.54mm) | Expansion GPIO |

## Building

### Prerequisites

- [PlatformIO](https://platformio.org/) (latest version)
- ESP32-S3 compatible board (DieselFire S3 PCB)
- USB-C cable

### Build Commands

```bash
# Install PlatformIO
pip install platformio

# Clone the repository
git clone https://github.com/yourusername/DieselFire-S3.git
cd DieselFire-S3

# Build firmware
pio run

# Upload to board
pio run --target upload

# Monitor serial output
pio device monitor --baud 115200
```

### Board Definition

The DieselFire S3 board is defined in `boards/dieselfire-s3.json`. To use it:

```bash
pio run -e dieselfire-s3
```

## Configuration

### WiFi Setup

On first boot, the device creates an access point:
- **SSID**: DieselFire-S3
- **Password**: thereisnospoon

Connect to this AP and open a browser to `192.168.4.1` to configure WiFi.

### MQTT Setup

Configure MQTT settings in the web interface or via the debug console:
- Server address
- Port (1883 for MQTT, 8883 for MQTT over TLS)
- Client ID
- Username/Password (if required)
- Topic prefix

### Sensor Calibration

**MQ-7 CO Sensor:**
1. Power on the device in clean air (outdoors)
2. Wait 48-72 hours for the sensor to stabilize
3. Use the web interface to run calibration
4. The R0 value is stored in non-volatile memory

**DS18B20 Temperature Probe:**
1. Connect the probe to J4
2. The probe will be auto-detected on boot
3. Set offset in settings if needed

## Hardware Design

### PCB Specifications

- **Size**: 80mm × 60mm
- **Layers**: 2-layer
- **Trace width**: 0.2mm signal, 0.5mm power, 1mm 12V
- **Via size**: 0.3mm hole / 0.6mm pad
- **Clearance**: 0.15mm minimum

### BOM

| Reference | Part | Package | Qty | Est. Cost |
|---|---|---|---|---|
| U1 | ESP32-S3-WROOM-1-N8R8 | QFN-44 module | 1 | ~$4.00 |
| U2 | 2.8" ILI9341 TFT + FPC | FPC connector | 1 | ~$3.50 |
| U3 | GT911 | QFN-24 4×4 | 1 | ~$0.80 |
| U4 | BME280 | QFN-8 2×2 | 1 | ~$1.50 |
| U5 | MQ-7 | JST-XH 4-pin | 1 | ~$2.00 |
| U6 | DS18B20 | JST-XH 3-pin | 1 | ~$1.50 |
| U7 | DS3231 | SOIC-16 | 1 | ~$2.00 |
| U8 | 74LCX125 | SOIC-14 | 1 | ~$0.30 |
| U9×2 | BSS138 | SOT-23 | 2 | ~$0.10 |
| U10 | MP2451 | SOT-23-6 | 1 | ~$0.50 |
| U11 | AP2112 | SOT-89-3 | 1 | ~$0.20 |
| J1 | USB-C-S6 | SMD | 1 | ~$0.30 |
| J2 | Terminal Block 2×3.81 | TB | 1 | ~$0.20 |
| J3-J5 | JST-XH connectors | 2.5mm | 3 | ~$0.50 |
| BAT1 | CR2032 Holder | — | 1 | ~$0.50 |
| H1,H2 | 2×10 Header | 2.54mm | 2 | ~$0.20 |
| D1,D2 | LED 0603 | 0603 | 2 | ~$0.05 |
| SW1,SW2 | Tactile 6×6 | 6×6mm | 2 | ~$0.10 |
| Various | Resistors/Caps | 0402/0603 | ~20 | ~$1.00 |
| **Total BOM** | | | | **~$19.00** |

**PCB fabrication:** ~$5-10 for 5 boards (JLCPCB/PCBWay, 2-layer, 80×60mm)

## Firmware Development

### Architecture

The firmware follows a modular architecture:

```
┌─────────────────────────────────────────────────────────────┐
│                     DieselFire S3                            │
│                                                              │
│  ┌─────────────┐  ┌──────────────┐  ┌──────────────────┐   │
│  │  Display    │  │   UI Layer   │  │  Settings/Config │   │
│  │  (LVGL)     │  │  (Screens)   │  │  (NVS Storage)   │   │
│  └─────────────┘  └──────────────┘  └──────────────────┘   │
│                                                              │
│  ┌─────────────┐  ┌──────────────┐  ┌──────────────────┐   │
│  │  Protocol   │  │  Blue Wire   │  │  Sensor Layer    │   │
│  │  Handling   │  │  State Machine│ │  (BME280, MQ-7)  │   │
│  └─────────────┘  └──────────────┘  └──────────────────┘   │
│                                                              │
│  ┌─────────────┐  ┌──────────────┐  ┌──────────────────┐   │
│  │  WiFi/HTTP  │  │  BLE/Bluetooth│ │  MQTT/OTA        │   │
│  └─────────────┘  └──────────────┘  └──────────────────┘   │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐   │
│  │                    ESP32-S3 Hardware                  │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

### Key Modules

| Module | Description | Location |
|---|---|---|
| **Display** | LVGL integration, ILI9341 driver, GT911 touch | `src/Display/` |
| **Protocol** | Blue wire protocol, state machine, Tx management | `src/Protocol/` |
| **RTC** | Real-time clock, timers, timer manager | `src/RTC/` |
| **Utility** | Sensors, NV storage, JSON, GPIO helpers | `src/Utility/` |
| **WiFi** | WiFiManager, web server, OTA, MQTT | `src/WiFi/` |
| **Bluetooth** | BLE implementation | `src/Bluetooth/` |
| **OLED** | Legacy OLED screens (to be replaced by LVGL) | `src/OLED/` |

### Development Workflow

1. **Create a feature branch**
   ```bash
   git checkout -b feature/new-screen
   ```

2. **Make changes**
   - Add/modify code in `src/`
   - Update documentation in `docs/`

3. **Build and test**
   ```bash
   pio run
   pio run --target upload
   pio device monitor --baud 115200
   ```

4. **Commit and push**
   ```bash
   git add .
   git commit -m "Add new screen for timer overview"
   git push origin feature/new-screen
   ```

5. **Create a pull request**

## Troubleshooting

### Board Won't Flash

- Check USB-C cable (must be data-capable, not charge-only)
- Hold BOOT button while pressing RESET
- Check COM port in `platformio.ini`

### Display Not Working

- Verify FPC connector is seated correctly
- Check SPI pin connections
- Verify PSRAM is detected (check serial output)

### BME280 Not Detected

- Check I2C address (0x76 or 0x77)
- Verify pull-up resistors (4.7kΩ on SDA/SCL)
- Run I2C scanner from debug console

### MQ-7 Readings Unstable

- Ensure 48-72 hour warmup period
- Run calibration in clean air
- Check voltage divider (10kΩ + 10kΩ)
- Verify ADC pin (IO4) is correct

### Blue Wire Communication Issues

- Verify level shifter connections (BSS138)
- Check Tx gate control (IO47)
- Verify UART0 pins (IO43/IO44)
- Check heater blue wire is connected correctly

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

Please ensure your code follows the existing style and includes appropriate documentation.

## Acknowledgments

DieselFire S3 is a fork of [Afterburner](https://gitlab.com/mrjones.id.au/bluetoothheater) by Ray Jones. The original Afterburner project was a groundbreaking open-source controller for Chinese diesel heaters, and this project builds upon that foundation.

Special thanks to:
- Ray Jones (Afterburner original author)
- James Clark (Afterburner co-author)
- The Afterburner community on GitLab
- The ESP32 and LVGL communities

## Disclaimer

DieselFire S3 is provided "as is" without warranty of any kind. The authors are not responsible for any damage caused by the use of this software or hardware. Diesel heaters produce carbon monoxide and other hazardous gases. Always ensure proper ventilation and installation.

Use at your own risk.
