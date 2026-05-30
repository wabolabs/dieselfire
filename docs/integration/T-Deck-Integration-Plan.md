# DieselFire S3 + T-Deck Integration Plan

## 1. Hardware Overview

### DieselFire S3 Board
| Spec | Value |
|---|---|
| Size | 80mm × 60mm × 1.6mm |
| MCU | ESP32-S3-WROOM-1-N8R8 (8MB Flash, 8MB PSRAM) |
| Display | 2.8" ILI9341 TFT (320×240) + GT911 touch |
| Connectors | USB-C, 2× JST-XH (3-pin), 1× JST-XH (4-pin), 2× Terminal Block |
| Expansion | 2× 2×10 headers (H1, H2) |
| Power | 12V input, MP2451 buck → 5V, AP2112 LDO → 3.3V |
| Sensors | BME280, DS3231 RTC, MQ-7 CO, DS18B20 temp probe |
| Interface | Blue Wire UART @ 25kbps (half-duplex) |

### TTGO T-Deck Board
| Spec | Value |
|---|---|
| Size | ~108mm × 62mm × 12mm (with case) |
| MCU | ESP32-P4 (higher performance) |
| Display | 4" IPS LCD (480×480) |
| Keyboard | Built-in membrane keyboard |
| Storage | MicroSD slot |
| Battery | Built-in LiPo battery connector |
| Connectors | USB-C, GPIO headers |
| Audio | Speaker + microphone |

## 2. Integration Architecture

### Mechanical Stack
```
┌────────────────────────────────────────────────────────────┐
│                    TOP VIEW                                  │
│                                                            │
│  ┌─────────────────────────────────────────────────────┐   │
│  │              T-Deck Case (Top Half)                  │   │
│  │  ┌───────────────────────────────────────────────┐   │   │
│  │  │  4" IPS Display (T-Deck)                       │   │   │
│  │  │  ┌─────────────────────────────────────────┐   │   │   │
│  │  │  │  Membrane Keyboard (T-Deck)              │   │   │   │
│  │  │  └─────────────────────────────────────────┘   │   │   │
│  │  └───────────────────────────────────────────────┘   │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                            │
│  ┌─────────────────────────────────────────────────────┐   │
│  │              DieselFire S3 Board                     │   │
│  │  ┌───────────────────────────────────────────────┐   │   │
│  │  │  2.8" TFT Display (front-facing through slot)  │   │   │
│  │  └───────────────────────────────────────────────┘   │   │
│  │                                                      │   │
│  │  J2 (12V IN)  ──▶  Terminal block accessible from    │   │
│  │                    rear panel                        │   │
│  │                                                      │   │
│  │  J3 (Blue Wire) ──▶  JST-XH connector accessible     │   │
│  │                       from rear panel                │   │
│  │                                                      │   │
│  │  J5 (MQ-7)    ──▶  JST-XH probe extends outside     │   │
│  │                       case for CO sensing            │   │
│  │                                                      │   │
│  │  J4 (DS18B20) ──▶  JST-XH probe extends outside     │   │
│  │                       case for heater temp           │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                            │
│  ┌─────────────────────────────────────────────────────┐   │
│  │              Bottom Plate                            │   │
│  │  ┌───────────────────────────────────────────────┐   │   │
│  │  │  Mounting Standoffs                            │   │   │
│  │  └───────────────────────────────────────────────┘   │   │
│  └─────────────────────────────────────────────────────┘   │
└────────────────────────────────────────────────────────────┘
```

### Electrical Integration

```
┌──────────────────────────────────────────────────────────────┐
│                    INTEGRATION SCHEMA                         │
│                                                              │
│  ┌──────────────┐      ┌──────────────────┐                  │
│  │  T-Deck      │      │  DieselFire S3   │                  │
│  │  (ESP32-P4)  │      │  (ESP32-S3)      │                  │
│  │              │      │                  │                  │
│  │  ┌─────────┐ │      │  ┌────────────┐  │                  │
│  │  │ 4" LCD  │ │      │  │ 2.8" TFT   │  │                  │
│  │  │ Display │ │      │  │ Display    │  │                  │
│  │  └─────────┘ │      │  └────────────┘  │                  │
│  │              │      │                  │                  │
│  │  ┌─────────┐ │      │  ┌────────────┐  │                  │
│  │  │Keyboard │ │      │  │ MQ-7 Probe │──┼──▶ Outside Case  │
│  │  └─────────┘ │      │  └────────────┘  │                  │
│  │              │      │                  │                  │
│  │  ┌─────────┐ │      │  ┌────────────┐  │                  │
│  │  │  SD     │ │      │  │ DS18B20    │──┼──▶ Outside Case  │
│  │  └─────────┘ │      │  └────────────┘  │                  │
│  │              │      │                  │                  │
│  │  ┌─────────┐ │      │  ┌────────────┐  │                  │
│  │  │Battery  │ │      │  │ 12V IN     │──┼──▶ Terminal Block│
│  │  └─────────┘ │      │  └────────────┘  │                  │
│  │              │      │                  │                  │
│  │  ┌─────────┐ │      │  ┌────────────┐  │                  │
│  │  │I2C Bus  │◀┼──────┼──│ BME280     │  │                  │
│  │  └─────────┘ │      │  └────────────┘  │                  │
│  │              │      │                  │                  │
│  │  ┌─────────┐ │      │  ┌────────────┐  │                  │
│  │  │UART     │◀┼──────┼──│ Blue Wire  │──┼──▶ Heater       │
│  │  └─────────┘ │      │  └────────────┘  │                  │
│  └──────────────┘      └──────────────────┘                  │
│                                                              │
│  Communication: I2C + UART between T-Deck and DieselFire S3  │
└──────────────────────────────────────────────────────────────┘
```

### Inter-Board Communication

| Signal | T-Deck | DieselFire S3 | Purpose |
|---|---|---|---|
| I2C SDA | GPIO | IO21 (shared) | BME280, DS3231 data to T-Deck |
| I2C SCL | GPIO | IO22 (shared) | I2C clock |
| UART TX | GPIO | IO43 | DieselFire → T-Deck status |
| UART RX | GPIO | IO44 | T-Deck → DieselFire commands |
| 3.3V | — | Pin 9 (H1/H2) | Power to T-Deck (optional) |
| GND | — | Pin 10 (H1/H2) | Common ground |

## 3. Case Design

### Required Features

1. **T-Deck Section (Top)**
   - 4" display window (clear acrylic or open)
   - Keyboard access (open front or membrane-compatible opening)
   - MicroSD slot access
   - USB-C port access (front or side)
   - Speaker/microphone openings

2. **DieselFire S3 Section (Middle)**
   - 2.8" display window (aligned to front)
   - Terminal block access (rear panel)
   - JST-XH connector access (rear panel)
   - Expansion header access (top or side)

3. **Probe Routing**
   - DS18B20 probe: 1-2m cable, routed through rear
   - MQ-7 probe: external mount, cable through rear
   - Blue wire: JST-XH connector on rear panel

4. **Mounting**
   - 4× M3 standoffs (80mm × 60mm spacing)
   - DieselFire S3 → T-Deck: via H1/H2 header connection or ribbon cable
   - Bottom plate for terminal block support

### Design Specifications

| Parameter | Value |
|---|---|
| Material | PETG or PLA (3D printed) |
| Wall Thickness | 1.2mm (3 perimeters) |
| Infill | 20% gyroid |
| T-Deck to DieselFire S3 Gap | 5-10mm (for ribbon cable) |
| Display Window | 2.8" TFT visible through 4" display area |
| Rear Panel | Removable for wiring access |
| Total Height | ~35mm (T-Deck 12mm + gap 10mm + DieselFire 1.6mm + standoffs 10mm) |

### 3D Model Structure

```
case/
├── top_cover.stl          # T-Deck top half with display window
├── middle_plate.stl       # DieselFire S3 mounting plate
├── bottom_plate.stl       # Base plate with standoff mounts
├── rear_panel.stl         # Removable rear panel with connector cutouts
├── terminal_block_mount.stl  # Terminal block support bracket
├── standoff_10mm.stl      # M3 standoff (10mm)
├── standoff_15mm.stl      # M3 standoff (15mm)
└── probe_cable_guide.stl  # Cable management for probes
```

## 4. Assembly Instructions

### Parts List

| Item | Qty | Description |
|---|---|---|
| T-Deck Board | 1 | Main controller with display/keyboard |
| DieselFire S3 Board | 1 | Heater controller |
| M3 Standoffs (10mm) | 4 | Board mounting |
| M3 Standoffs (15mm) | 4 | T-Deck to DieselFire gap |
| M3 Screws (6mm) | 8 | Board attachment |
| Ribbon Cable (20-pin) | 1 | Inter-board connection |
| DS18B20 Probe | 1 | External temp sensor (1-2m) |
| MQ-7 Sensor | 1 | CO sensor module |
| JST-XH Cables | 3 | Sensor connections |
| Terminal Block | 1 | 12V power input |
| 3D Printed Case Parts | 1 set | All STL files |

### Assembly Steps

1. **Print Case Parts**
   - Print all STL files in PETG (recommended for heat resistance)
   - Remove supports, sand rough edges

2. **Mount DieselFire S3 Board**
   - Attach 4× M3 standoffs (10mm) to bottom plate
   - Secure DieselFire S3 board with M3 screws
   - Connect ribbon cable to H1/H2 headers

3. **Mount T-Deck Board**
   - Attach 4× M3 standoffs (15mm) to middle plate
   - Secure T-Deck board with M3 screws
   - Connect ribbon cable to T-Deck GPIO headers

4. **Install Case**
   - Place T-Deck section into top cover
   - Align DieselFire S3 section in middle plate
   - Close with rear panel (terminal block accessible)

5. **Connect Probes**
   - Route DS18B20 probe through rear panel
   - Route MQ-7 probe through rear panel
   - Connect Blue Wire to heater via JST-XH

6. **Power Up**
   - Connect 12V to terminal block
   - Verify USB-C connection for debugging
   - Flash firmware to both boards

## 5. Firmware Integration

### T-Deck Firmware
- Main UI on 4" display
- Keyboard input handling
- WiFi/BLE connectivity
- SD card logging
- I2C/UART bridge to DieselFire S3

### DieselFire S3 Firmware
- Heater control logic (PWM fan, relay)
- Blue Wire protocol (25kbps half-duplex)
- Sensor reading (BME280, DS18B20, MQ-7)
- RTC timekeeping (DS3231)
- I2C/UART responder to T-Deck

### Communication Protocol

```
T-Deck ←── I2C ──▶ DieselFire S3 (sensor data)
T-Deck ←── UART ──▶ DieselFire S3 (heater status)
T-Deck ──▶ UART ──▶ DieselFire S3 (control commands)
```

## 6. Wiring Diagram

```
                    ┌──────────────────────────────────┐
                    │          REAR PANEL              │
                    │                                  │
                    │  ┌─────────┐  ┌───────────────┐  │
                    │  │  J2     │  │     J3        │  │
                    │  │ 12V IN  │  │ Blue Wire     │  │
                    │  │(TermBlk)│  │(JST-XH 3pin)  │  │
                    │  └─────────┘  └───────────────┘  │
                    │                                  │
                    │  ┌─────────┐  ┌───────────────┐  │
                    │  │  J4     │  │     J5        │  │
                    │  │DS18B20  │  │   MQ-7        │  │
                    │  │(JST-XH) │  │(JST-XH 4pin)  │  │
                    │  └─────────┘  └───────────────┘  │
                    └────────┬─────────────────────────┘
                             │
                    ┌────────▼─────────────────────────┐
                    │       DieselFire S3 Board        │
                    │                                  │
                    │  ┌────────────────────────────┐  │
                    │  │  H1 (2×10 Header)          │  │
                    │  │  ──▶ Ribbon Cable ──▶ H2   │  │
                    │  └────────────────────────────┘  │
                    │                                  │
                    │  ┌────────────────────────────┐  │
                    │  │  U1 ESP32-S3               │  │
                    │  │  U2 2.8" TFT Display       │  │
                    │  │  U4 BME280 (on-board)      │  │
                    │  │  U7 DS3231 (on-board)      │  │
                    │  └────────────────────────────┘  │
                    └────────┬─────────────────────────┘
                             │
                    ┌────────▼─────────────────────────┐
                    │          T-Deck Board            │
                    │                                  │
                    │  ┌────────────────────────────┐  │
                    │  │  4" IPS Display            │  │
                    │  │  Membrane Keyboard         │  │
                    │  │  ESP32-P4 MCU              │  │
                    │  │  MicroSD Slot              │  │
                    │  │  Battery Connector         │  │
                    │  └────────────────────────────┘  │
                    └──────────────────────────────────┘
```

## 7. Next Steps

1. **3D Model Creation**
   - Create STEP/STL files for all case parts
   - Verify fit with actual T-Deck and DieselFire S3 boards
   - Test print first iteration

2. **Firmware Development**
   - Implement I2C/UART bridge on both boards
   - Test sensor data flow T-Deck ← DieselFire S3
   - Test control commands T-Deck → DieselFire S3

3. **Prototype Assembly**
   - Print case parts
   - Assemble boards with ribbon cable
   - Connect all probes and test

4. **Refinement**
   - Adjust case dimensions based on prototype
   - Optimize firmware communication protocol
   - Final enclosure design for production
