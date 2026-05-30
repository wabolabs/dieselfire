# DieselFire S3 - Design Document

## Project Overview

DieselFire S3 is a modernized controller for generic Chinese diesel heaters, forked from the original [Afterburner](https://gitlab.com/mrjones.id.au/bluetoothheater) project. This document describes the complete hardware and firmware design.

## Block Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                        DIESELFIRE S3                                │
│                                                                     │
│  ┌──────────┐    ┌──────────────┐    ┌──────────────────────────┐  │
│  │ 12V IN   │───▶│ MP2451 Buck  │───▶│ 5V Rail                  │  │
│  │ (J2)     │    │ 12V→5V       │    │                          │  │
│  └──────────┘    └──────────────┘    │  ┌────────────────────┐  │  │
│                                      │  │ AP2112 LDO         │  │  │
│                                      │  │ 5V→3.3V            │  │  │
│                                      │  └────────┬───────────┘  │  │
│                                      └───────────┼─────────────┘  │  │
│                                                  │                 │
│  ┌──────────┐    ┌──────────────┐    ┌───────────┼──────────┐    │  │
│  │ USB-C    │    │ ESP32-S3     │    │           │          │    │  │
│  │ (J1)     │───▶│ WROOM-1-N8R8 │    │  SPI      │  I2C     │    │  │
│  └──────────┘    │ (U1)         │    │           │          │    │  │
│                  │ 8MB Flash    │    │  ┌──────┐  │  ┌────┐  │    │  │
│                  │ 8MB PSRAM    │    │  │ILI93│  │  │BME │  │    │  │
│                  └──────┬───────┘    │  │ 41  │  │  │280 │  │    │  │
│                         │            │  └──────┘  │  └────┘  │    │  │
│                         │            │           │  │GT911 │  │    │  │
│                         │            │  ┌──────┐  │  └────┘  │    │  │
│                         │            │  │74LCX│  │  │DS3231│  │    │  │
│                         │            │  │125  │  │  └────┘  │    │  │
│                         │            │  └──┬──┘  └──────────┘    │  │
│                         │            │     │                     │  │
│                         │            │  BT│WIRE                 │  │
│                         │            │  UART                     │  │
│                         │            │     │                     │  │
│                         │            │  MQ-7                    │  │
│                         │            │  (J5)                    │  │
│                         │            │     │                     │  │
│                         │            │  DS18B20                 │  │
│                         │            │  (J4)                    │  │
│                         │            └──────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────┘
```

## Power System

### Power Flow

```
12V IN (J2) ──┬───▶ MP2451 Buck ──▶ 5V Rail
                │
                └───▶ Reverse Polarity Protection (P-channel MOSFET)
                      │
                      └───▶ TVS Diode (SMAJ12A)
                              │
                              └───▶ Ferrite Bead (600Ω @ 100MHz)
                                      │
                                      └───▶ MP2451 VIN
```

### Regulators

| Regulator | Input | Output | Current | Package |
|---|---|---|---|---|
| MP2451 | 12V | 5V | 2A | SOT-23-6 |
| AP2112 | 5V | 3.3V | 1A | SOT-89-3 |

### Protection

- **Reverse Polarity:** P-channel MOSFET (SI2301)
- **Surge Protection:** TVS diode (SMAJ12A)
- **EMI Filtering:** Ferrite bead (600Ω @ 100MHz)
- **Decoupling:** 100µF + 0.1µF per rail

### Current Budget

| Component | Typical | Peak |
|---|---|---|
| ESP32-S3 (WiFi on) | 80-250mA | ~500mA |
| Display + backlight | 20-50mA | ~100mA |
| MQ-7 sensor | 15mA | 150mA (heating pulse) |
| Sensors (BME280, DS3231, GT911) | <5mA | <10mA |
| **Total** | **~150mA** | **~800mA** |

## Communication Interfaces

### Blue Wire (Heater Communication)

The "blue wire" is the half-duplex serial interface used by Chinese diesel heaters.

| Parameter | Value |
|---|---|
| Baud Rate | 25000 baud |
| Data Format | 8N1 |
| Logic Level | 5V (heater) / 3.3V (ESP32-S3) |
| Interface | UART0 (IO43/TX, IO44/RX) |

### Level Shifting

Bidirectional level shifting is required for the blue wire interface:

```
ESP32-S3 (3.3V) ──┬── BSS138 ──┬── Heater Blue Wire (5V)
                   │            │
                   └── BSS138 ──┘
```

### Tx Gate Control

A 74LCX125 buffer controls transmission to prevent feedback:

```
ESP32-S3 IO43 (TX) ──▶ 74LCX125 Input ──▶ BSS138 ──▶ Heater Blue Wire
                       │
ESP32-S3 IO47 (Tx Gate) ──▶ 74LCX125 Enable
```

### WiFi

- **Standard:** 802.11 b/g/n
- **Frequency:** 2.4 GHz
- **Modes:** STA (connect to existing network) or AP (captive portal)
- **Antenna:** Integrated on ESP32-S3 module

### Bluetooth

- **Standard:** BLE 5
- **Frequency:** 2.4 GHz
- **Profile:** Serial Port Profile (SPP) over BLE
- **Connection:** Mobile app or web interface

### USB

- **Type:** USB-C
- **Function:** Programming, debug, power
- **Protocol:** Native USB (no external USB-serial chip needed)
- **Baud Rate:** 115200 (debug console)

## Display Interface

### ILI9341 TFT Display

| Parameter | Value |
|---|---|
| Resolution | 320 × 240 pixels |
| Interface | SPI (4-wire) |
| Clock Speed | 40 MHz (write), 6 MHz (read) |
| Color Depth | 16-bit RGB565 |
| Backlight | LED (PWM controllable) |

### Pin Mapping

| Signal | ESP32-S3 GPIO | Notes |
|---|---|---|
| MOSI | IO18 | HSPI_MOSI |
| SCK | IO17 | HSPI_SCK |
| MISO | IO16 | HSPI_MISO (optional) |
| CS | IO15 | Chip select |
| DC | IO14 | Data/command select |
| RESET | IO13 | Display reset |
| BACKLIGHT | IO12 | PWM controlled |

### GT911 Capacitive Touch

| Signal | ESP32-S3 GPIO | Notes |
|---|---|---|
| SDA | IO21 | I2C data (shared bus) |
| SCL | IO10 | I2C clock |
| INT | IO9 | Touch interrupt |
| RST | IO8 | Touch reset |

### LVGL Graphics Library

| Parameter | Value |
|---|---|
| Version | 9.5.0 |
| Display Driver | esp_lcd (ILI9341) |
| Touch Driver | esp_lcd_touch_gt911 |
| Buffer Location | PSRAM (8MB) |
| Buffer Size | 320×240×2 bytes (153,600 bytes) |
| Double Buffering | Yes |
| DMA Support | Yes (SPI DMA) |

## Sensor Interface

### BME280 (Temperature, Humidity, Pressure)

| Parameter | Value |
|---|---|
| I2C Address | 0x76 (SDO → GND) |
| Temp Accuracy | ±1.0°C |
| Humidity Accuracy | ±3% RH |
| Pressure Accuracy | ±1 hPa |
| Interface | I2C (shared bus) |
| Sampling Mode | Forced |

### MQ-7 (Carbon Monoxide)

| Parameter | Value |
|---|---|
| Interface | Analog + Digital |
| AOUT | GPIO4 (ADC1, 12-bit) |
| DOUT | GPIO5 (GPIO input) |
| Power | 5V, ~75mA typical |
| Response Time | ~5 seconds |
| Voltage Divider | 10kΩ + 10kΩ (5V → 2.5V) |

### DS18B20 (External Temperature Probe)

| Parameter | Value |
|---|---|
| Interface | OneWire |
| GPIO | IO45 |
| Pull-up | 4.7kΩ |
| Accuracy | ±0.5°C |
| Range | -55°C to +125°C |
| Connector | JST-XH 3-pin |

### DS3231 (RTC)

| Parameter | Value |
|---|---|
| I2C Address | 0x68 |
| Interface | I2C (shared bus) |
| Battery | CR2032 |
| Accuracy | ±2 ppm (~1 minute/year) |
| Alarm | 1 per day |
| Connector | On-board (BAT1) |

## Connector Pinouts

### J1: USB-C

| Pin | Signal | Notes |
|---|---|---|
| A1, B12 | GND | Ground |
| A9, B10 | VBUS | 5V power |
| A7, B8 | CC1/CC2 | USB-C detection (5.1kΩ to GND) |
| A24, B17 | D+ | USB data + (IO19) |
| A25, B18 | D- | USB data - (IO20) |
| A17, B24 | SHIELD | Ground |

### J2: 12V Power Input (Terminal Block)

| Pin | Signal | Notes |
|---|---|---|
| 1 | 12V IN | Via reverse polarity protection |
| 2 | GND | Ground |

### J3: Blue Wire to Heater (JST-XH 3-pin)

| Pin | Signal | Notes |
|---|---|---|
| 1 | Blue Wire | Heater data line |
| 2 | 12V SENSE | Heater 12V sense |
| 3 | GND | Ground |

### J4: DS18B20 External Probe (JST-XH 3-pin)

| Pin | Signal | Notes |
|---|---|---|
| 1 | VCC | 3.3V |
| 2 | DATA | OneWire data (4.7kΩ pull-up) |
| 3 | GND | Ground |

### J5: MQ-7 CO Sensor (JST-XH 4-pin)

| Pin | Signal | Notes |
|---|---|---|
| 1 | VCC | 5V |
| 2 | GND | Ground |
| 3 | AOUT | Analog output (via voltage divider) |
| 4 | DOUT | Digital output |

### H1, H2: Expansion Headers (2×10, 2.54mm pitch)

**H1:**
| Pin | Signal | Pin | Signal |
|---|---|---|---|
| 1 | GPIO33 | 2 | GPIO34 |
| 3 | GPIO35 | 4 | GPIO36 |
| 5 | GPIO37 | 6 | GPIO38 |
| 7 | GPIO39 | 8 | GPIO40 |
| 9 | 3.3V | 10 | GND |
| 11 | GND | 12 | NC |
| 13 | NC | 14 | NC |
| 15 | NC | 16 | NC |
| 17 | NC | 18 | NC |
| 19 | NC | 20 | NC |

**H2:**
| Pin | Signal | Pin | Signal |
|---|---|---|---|
| 1 | GPIO41 | 2 | GPIO42 |
| 3 | GPIO43 | 4 | GPIO44 |
| 5 | GPIO45 | 6 | GPIO46 |
| 7 | GPIO47 | 8 | GPIO48 |
| 9 | 3.3V | 10 | GND |
| 11 | GND | 12 | NC |
| 13 | NC | 14 | NC |
| 15 | NC | 16 | NC |
| 17 | NC | 18 | NC |
| 19 | NC | 20 | NC |

## PCB Layout Guidelines

### Layer Stackup

| Layer | Description |
|---|---|
| Layer 1 (Top) | Signal traces, SMD components, silkscreen |
| Layer 2 (Bottom) | Ground plane, some signal traces, SMD components |

### Trace Widths

| Signal | Minimum Width | Notes |
|---|---|---|
| 12V Power | 1.0mm | Carries up to 2A |
| 5V Power | 0.5mm | Carries up to 1A |
| 3.3V Power | 0.3mm | Carries up to 500mA |
| Signal | 0.2mm | Standard signal traces |

### Via Requirements

| Parameter | Value |
|---|---|
| Hole Size | 0.3mm |
| Pad Size | 0.6mm |
| Clearance | 0.3mm |

### Clearance

| Parameter | Value |
|---|---|
| Minimum Clearance | 0.15mm |
| Minimum Copper-to-Copper | 0.2mm |
| Minimum Silkscreen-to-Copper | 0.2mm |

## Schematic Notes

### Power Rails

- **12V:** From J2 (terminal block), protected by reverse polarity circuit
- **5V:** From MP2451 buck converter, decoupled with 100µF + 0.1µF
- **3.3V:** From AP2112 LDO, decoupled with 10µF + 0.1µF

### I2C Bus

All I2C devices share the same bus (SDA: IO21, SCL: IO22):
- BME280 (0x76)
- GT911 touch (0x5D)
- DS3231 RTC (0x68)
- SCD41 CO2 (0x62, optional)

Pull-up resistors: 4.7kΩ on SDA and SCL to 3.3V.

### SPI Bus

The ILI9341 display uses HSPI:
- MOSI: IO18
- SCK: IO17
- MISO: IO16 (optional)
- CS: IO15
- DC: IO14
- RST: IO13
- BL: IO12

### UART for Blue Wire

UART0 is used for blue wire communication:
- TX: IO43
- RX: IO44
- Baud Rate: 25000
- Data Format: 8N1

Tx gate control via IO47 to 74LCX125 buffer.

### ADC

MQ-7 analog output connected to ADC1:
- Pin: IO4
- Resolution: 12-bit
- Attenuation: 11dB (0-3.3V range)
- Voltage Divider: 10kΩ + 10kΩ (5V → 2.5V)

### OneWire

DS18B20 external probe:
- Pin: IO45
- Pull-up: 4.7kΩ to 3.3V
- Protocol: Dallas OneWire

## Testing Points

| Test Point | Signal | Notes |
|---|---|---|
| TP1 | 3.3V | ESP32-S3 supply |
| TP2 | 5V | Buck converter output |
| TP3 | 12V | Power input |
| TP4 | GND | Ground |
| TP5 | IO43 | UART0 TX (blue wire) |
| TP6 | IO44 | UART0 RX (blue wire) |
| TP7 | IO18 | SPI MOSI (display) |
| TP8 | IO21 | I2C SDA (sensors) |
| TP9 | IO22 | I2C SCL (sensors) |

## Revision History

| Version | Date | Changes |
|---|---|---|
| 1.0 | 2024-01-01 | Initial design |

## References

- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [ILI9341 Datasheet](https://www.adafruit.com/datasheets/ILI9341.pdf)
- [BME280 Datasheet](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds001.pdf)
- [MQ-7 Datasheet](https://www.sparkfun.com/datasheets/Sensors/MQ-7.pdf)
- [DS3231 Datasheet](https://datasheets.maximintegrated.com/en/ds/DS3231.pdf)
- [GT911 Datasheet](https://github.com/goodtft/GT911/blob/master/GT911_Datasheet.pdf)
