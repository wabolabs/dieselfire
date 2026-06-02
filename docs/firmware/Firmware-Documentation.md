# DieselFire - Firmware Documentation

## Architecture Overview

DieselFire firmware is built on the ESP32-S3 with ESP-IDF v5.x framework. The architecture follows a modular design with clear separation of concerns.

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     DieselFire                            │
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

## Module Description

### Display Module (`src/Display/`)

Handles LVGL integration, display driver, and touch input.

**Key Files:**
- `LVGLDisplay.cpp/.h` - LVGL initialization and display management
- `IL9341Driver.cpp/.h` - ILI9341 SPI display driver
- `GT911Touch.cpp/.h` - GT911 capacitive touch driver
- `LVGLScreens/` - Individual screen implementations

**Initialization:**
```cpp
// In LVGLDisplay::begin()
lv_init();
lv_display_t* disp = lvgl_port_init();
lv_indev_t* touch = lvgl_port_add_touch();
```

### Protocol Module (`src/Protocol/`)

Handles blue wire communication with the diesel heater.

**Key Files:**
- `Protocol.cpp/.h` - Protocol frame handling
- `TxManage.cpp/.h` - Transmission management
- `BlueWireTask.cpp/.h` - Blue wire state machine task
- `SmartError.cpp/.h` - Error detection and handling

**Blue Wire State Machine:**
1. **Idle** - Wait for heater activity
2. **OEMCtrlRx** - Receive OEM controller frame (if present)
3. **HeaterRx1** - Receive heater response to OEM
4. **TxStart** - Start transmission
5. **TxInterval** - Transmit data
6. **HeaterRx2** - Receive heater response to BTC
7. **ExchangeComplete** - Complete exchange

### RTC Module (`src/RTC/`)

Handles real-time clock and timer management.

**Key Files:**
- `Clock.cpp/.h` - DS3231 RTC driver
- `Timers.cpp/.h` - Timer management
- `TimerManager.cpp/.h` - Timer scheduling
- `RTCStore.cpp/.h` - RTC data storage

### Utility Module (`src/Utility/`)

General utility classes for sensors, storage, and helpers.

**Key Files:**
- `TempSense.cpp/.h` - Temperature sensor abstraction (BME280 + DS18B20)
- `MQCOSensor.cpp/.h` - MQ-7 CO sensor driver
- `NVStorage.cpp/.h` - Non-volatile storage
- `BTC_JSON.cpp/.h` - JSON data handling
- `FuelGauge.cpp/.h` - Fuel consumption tracking
- `HourMeter.cpp/.h` - Run time and glow time tracking

### WiFi Module (`src/WiFi/`)

Handles WiFi connectivity, web server, and OTA updates.

**Key Files:**
- `BTCWifi.cpp/.h` - WiFi connection management
- `BTCWebServer.cpp/.h` - Web server with WebSocket
- `BTCota.cpp/.h` - OTA update handling
- `ABMQTT.cpp/.h` - MQTT client

### Bluetooth Module (`src/Bluetooth/`)

Handles BLE communication for mobile app control.

**Key Files:**
- `BluetoothESP32.cpp/.h` - ESP32 native BLE implementation
- `BluetoothAbstract.cpp/.h` - Abstract Bluetooth interface

### OLED Module (`src/OLED/`)

Legacy OLED screens (to be replaced by LVGL).

**Note:** This module is being phased out. New screens should use LVGL.

## Build Configuration

### PlatformIO Setup

**platformio.ini:**
```ini
[env:dieselfire]
platform = espressif32@6.12.0
board = dieselfire
framework = espidf
board_build.partitions = default_8MB.csv
build_flags =
    -DBOARD_HAS_PSRAM
    -DHTTPS_LOGLEVEL=2
    -DCORE_DEBUG_LEVEL=5
lib_deps =
    lvgl/lvgl@^9.5.0
    espressif/esp_lcd@^2.0.0
    espressif/esp_lvgl_port@^2.0.0
    espressif/esp_lcd_touch_gt911@^1.2.0
    adafruit/Adafruit BME280 Library@^2.3.0
    adafruit/RTClib@^2.1.3
    arduinojson/ArduinoJson@^7.2.0
    wmaz/WiFiManager@^2.0.17
    marvinroger/async-mqtt-client@^0.9.0
upload_speed = 921600
monitor_speed = 115200
```

### Board Definition

**boards/dieselfire.json:**
```json
{
  "build": {
    "core": "esp32",
    "cpu": "esp32s3",
    "f_cpu": "240000000L",
    "f_flash": "80000000L",
    "flash_mode": "qio",
    "mcu": "esp32s3",
    "variant": "esp32s3",
    "partitions": "default_8MB.csv",
    "extra_inc": "sdkconfig.h"
  },
  "connectivity": ["wifi", "bluetooth"],
  "debug": {
    "openocd_target": "esp32s3.cfg"
  },
  "frameworks": ["arduino", "espidf"],
  "name": "DieselFire",
  "upload": {
    "flash_size": "8MB",
    "max_ram_size": "327680",
    "max_size": 8388608,
    "require_upload_port": true,
    "speed": 921600
  },
  "url": "https://github.com/yourusername/DieselFire",
  "vendor": "DieselFire"
}
```

## Sensor Calibration

### MQ-7 CO Sensor

**Calibration Procedure:**
1. Power on device in clean air (outdoors)
2. Wait 48-72 hours for sensor stabilization
3. Run calibration routine via web interface or debug console
4. R0 value is stored in NVS

**Calibration Code:**
```cpp
// In MQCOSensor::calibrate()
float rs = readRs();  // Read sensor resistance
R0 = rs / pow(PPM / a, 1.0 / b);  // Calculate R0
saveR0ToNVS();  // Store in non-volatile storage
```

### DS18B20 Temperature Probe

**Offset Calibration:**
1. Compare reading with known accurate thermometer
2. Set offset in settings: `TempSensor.setOffset(idx, offset)`
3. Offset is stored in NVS

## Debugging

### Serial Console

Access via USB-C at 115200 baud:
```bash
pio device monitor --baud 115200
```

**Debug Commands:**
- `+` - Request heater ON
- `-` - Request heater OFF
- `r` - Reboot device
- `s` - Show sensor readings
- `m` - Configure MQTT
- `j` - Toggle JSON reporting

### I2C Scanner

Scan I2C bus for connected devices:
```cpp
// In debug console
i2c_scan
```

Expected output:
```
I2C Scanner:
  Address: 0x76 - BME280
  Address: 0x5D - GT911
  Address: 0x68 - DS3231
```

### LVGL Debug

Enable LVGL debug output:
```cpp
// In platformio.ini
build_flags =
    -DLV_LOG_LEVEL=LV_LOG_LEVEL_TRACE
    -DLV_LOG_PRINTF=1
```

## OTA Update

### Over-the-Air Firmware Updates

**Via Web Interface:**
1. Connect to device via WiFi
2. Navigate to Settings → Firmware Update
3. Upload .bin file
4. Device reboots with new firmware

**Via Serial:**
```bash
pio run --target upload
```

**Via OTA:**
```bash
pio run --target upload --upload-port 192.168.1.100
```

## Memory Layout

### Flash Partition

For 8MB flash:
```
# Name,   Type, SubType, Offset,    Size, Flags
nvs,      data, nvs,     0x9000,    0x5000,
otadata,  data, ota,     0xe000,    0x2000,
app0,     app,  ota_0,   0x10000,   0x1E0000,
app1,     app,  ota_1,   0x1F0000,  0x1E0000,
spiffs,   data, spiffs,  0x3D0000,  0x30000,
coredump, data, coredump,0x6D0000,  0x10000,
fat,      data, fat,     0x6E0000,  0x720000,
```

### PSRAM Usage

| Component | Size | Notes |
|---|---|---|
| LVGL Display Buffer | 153,600 bytes | 320×240×2 (double buffered) |
| LVGL Font Cache | 32,768 bytes | Cached font glyphs |
| LVGL Widget Objects | 65,536 bytes | Screen widgets and elements |
| WebSocket Buffers | 32,768 bytes | JSON data buffering |
| **Total** | **~284,672 bytes** | Well within 8MB PSRAM |

## Task Priorities

| Task | Priority | Stack Size | Notes |
|---|---|---|---|
| Blue Wire Task | 4 | 4096 bytes | Highest priority, real-time |
| Arduino Loop | 3 | 2048 bytes | Main application loop |
| LVGL Task | 2 | 2048 bytes | Display updates |
| WiFi Task | 1 | 4096 bytes | WiFi management |
| MQTT Task | 1 | 2048 bytes | MQTT communication |
| Watchdog Task | 5 | 1024 bytes | Highest priority, watchdog |

## Error Handling

### Watchdog

Hardware watchdog (TWDT) is enabled:
```cpp
// In setup()
esp_task_wdt_init(TWDT_TIMEOUT_S, true);
esp_task_wdt_add(NULL);
```

### Brownout Detector

Brownout detector is enabled:
```cpp
// In setup()
esp_pm_config_t pm_config = {
    .max_freq_mhz = 240,
    .min_freq_mhz = 40,
    .light_sleep_enable = true
};
esp_pm_configure(&pm_config);
```

### NVS Error Handling

Non-volatile storage errors are handled gracefully:
```cpp
// In NVStorage::init()
nvs_handle_t handle;
esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
if (err != ESP_OK) {
    // Handle error, possibly erase NVS
    nvs_flash_erase();
    nvs_flash_init();
}
```

## Contributing to Firmware

### Adding a New Screen

1. Create new file in `src/Display/LVGLScreens/`
2. Implement `CLVGLScreen` base class
3. Add to screen manager
4. Register in menu system

### Adding a New Sensor

1. Create new driver in `src/Utility/`
2. Implement `ISensor` interface
3. Add to sensor manager
4. Update NVS storage if needed

### Testing

Run tests with:
```bash
pio test --environment dieselfire
```

## References

- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/)
- [LVGL Documentation](https://docs.lvgl.io/)
- [PlatformIO Documentation](https://docs.platformio.org/)
- [DieselFire Documentation](https://dieselfire.wabo.cc/-/wikis/home)
