# DieselFire - Hardware Documentation

## PCB Design

### Board Specifications

| Parameter | Value |
|---|---|
| Dimensions | 100mm × 100mm |
| Layers | 2-layer |
| Board Thickness | 1.6mm |
| Copper Weight | 1oz (35µm) |
| Surface Finish | HASL (Lead-free) |
| Silkscreen | White |
| Solder Mask | Green |

### Stackup

| Layer | Description |
|---|---|
| Layer 1 (Top) | Signal traces, SMD components, silkscreen |
| Layer 2 (Bottom) | Ground plane, some signal traces, SMD components |

### Design Rules

| Parameter | Value |
|---|---|
| Trace Width (Signal) | 0.2mm minimum |
| Trace Width (Power) | 0.5mm minimum |
| Trace Width (12V) | 1.0mm minimum |
| Via Hole Size | 0.3mm |
| Via Pad Size | 0.6mm |
| Clearance | 0.15mm minimum |
| Copper-to-Copper | 0.2mm minimum |
| Silkscreen-to-Copper | 0.2mm minimum |

## Schematic

The schematic is divided into the following sections:

### 1. Power Supply

- **Input:** 12V DC via terminal block (J2)
- **Protection:** Reverse polarity (P-channel MOSFET), TVS diode, ferrite bead
- **Regulation:** MP2451 buck (12V→5V), AP2112 LDO (5V→3.3V)
- **Decoupling:** 100µF + 0.1µF per rail

### 2. Microcontroller

- **MCU:** ESP32-S3-WROOM-1-N8R8 (U1)
- **Flash:** 8MB
- **PSRAM:** 8MB
- **USB:** Native USB-C (IO19/IO20)

### 3. Display

- **TFT:** 2.8" ILI9341, 320×240 (FPC into U11, an FH12-18S 0.5mm connector)
- **Touch:** GT911 capacitive (U10, on the main board)
- **Interface:** SPI (IO16-18), I2C (IO10, IO21)
- **Mounting:** The bare panel mounts to the **inside of the case lid** facing
  the user (active area 43.2 × 57.6 mm portrait, glass 50 × 69.2 mm). The PCB
  sits behind it; the FPC tail folds down to U11. A documentation keepout for
  the panel is drawn on the board's `Dwgs.User` layer (see `DISP_*` in
  `kicad/pipeline.py`) so no tall part lands under the display.

### 3a. User Buttons

- **SW1/SW2:** right-angle (side-actuated) tactile switches on the front board
  edge; plungers face the front wall so they're pressed from the **side of the
  case**, keeping the front face clear for the touchscreen.

### 4. Sensors

- **BME280:** Temperature, humidity, pressure (U4)
- **DS3231:** RTC with battery backup (U7)
- **MQ-7:** CO sensor (external via J5)
- **DS18B20:** External temp probe (external via J4)

### 5. Blue Wire Interface

- **UART:** UART0 @ 25kbps (IO43/IO44)
- **Level Shifting:** BSS138 ×2 (U8)
- **Tx Gate:** 74LCX125 (U6)

### 6. Expansion

- **Headers:** H1, H2 (2×10, 2.54mm pitch)
- **GPIO:** 16 available GPIO pins

## PCB Layout

### Component Placement

```
┌──────────────────────────────────────────────────────────────┐
│  TOP VIEW (80mm × 60mm)                                      │
│                                                              │
│  ┌────────────┐  ┌──────────────────────────────────────┐   │
│  │  J2        │  │  J1 (USB-C)                          │   │
│  │  12V IN    │  │                                      │   │
│  └────────────┘  └──────────────────────────────────────┘   │
│                                                              │
│  ┌──────────────┐  ┌────────────────────────────────────┐   │
│  │  U10 AP2112  │  │  U1 ESP32-S3                       │   │
│  │  5V→3.3V LDO │  │  (center, large QFN)               │   │
│  └──────────────┘  └────────────┬───────────────────────┘   │
│                                 │                            │
│  ┌──────────────┐  ┌────────────┴───────────────────────┐   │
│  │  U9 MP2451   │  │  U2 TFT Display (FPC connector)    │   │
│  │  12V→5V Buck │  │  (top edge, above ESP32-S3)        │   │
│  └──────────────┘  └────────────────────────────────────┘   │
│                                                              │
│  ┌──────────┐  ┌──────────────────────────────────────┐    │
│  │  U4      │  │  J3 (Blue Wire)                      │    │
│  │  BME280  │  │  (right edge)                        │    │
│  └──────────┘  └──────────────────────────────────────┘    │
│                                                              │
│  ┌──────────┐  ┌──────────────────────────────────────┐    │
│  │  U7      │  │  J4 (DS18B20)                        │    │
│  │  DS3231  │  │  (bottom edge)                       │    │
│  └──────────┘  └──────────────────────────────────────┘    │
│                                                              │
│  ┌──────────┐  ┌──────────────────────────────────────┐    │
│  │  U8×2    │  │  J5 (MQ-7)                           │    │
│  │  BSS138  │  │  (bottom-right)                      │    │
│  └──────────┘  └──────────────────────────────────────┘    │
│                                                              │
│  ┌──────────┐  ┌──────────────────────────────────────┐    │
│  │  U6      │  │  H1,H2 + SW1,SW2, D1,D2             │    │
│  │  74LCX125│  │  (bottom-left)                       │    │
│  └──────────┘  └──────────────────────────────────────┘    │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  BAT1 (CR2032)                                       │   │
│  │  (corner, near edge)                                 │   │
│  └──────────────────────────────────────────────────────┘   │
└──────────────────────────────────────────────────────────────┘
```

### Key Layout Considerations

1. **Antenna Clearance:** Keep ground pour away from top edge (ESP32-S3 antenna)
2. **Power Traces:** 12V and 5V traces should be wide (≥1mm)
3. **SPI Traces:** Keep display SPI traces short (≤50mm)
4. **Blue Wire Traces:** Shield UART traces from noise
5. **MQ-7 Power:** Separate 5V rail for MQ-7 to avoid noise
6. **I2C Pull-ups:** 4.7kΩ on SDA and SCL

## Assembly

### BOM (Bill of Materials)

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

### Assembly Order

1. **Solder small SMD components first** (resistors, capacitors, LEDs)
2. **Solder ICs** (BSS138, MP2451, AP2112, 74LCX125, BME280, GT911, DS3231)
3. **Solder ESP32-S3 module** (U1) - use reflow oven or hot air
4. **Solder USB-C connector** (J1)
5. **Solder terminal block** (J2)
6. **Solder JST-XH connectors** (J3-J5)
7. **Solder headers** (H1, H2)
8. **Install CR2032 battery** (BAT1)
9. **Install TFT display** (U2) - slide into FPC connector
10. **Test and debug**

### Testing Checklist

- [ ] Measure 12V input at J2
- [ ] Measure 5V at MP2451 output
- [ ] Measure 3.3V at AP2112 output
- [ ] Check for short circuits between 3.3V and GND
- [ ] Check for short circuits between 5V and GND
- [ ] Check for short circuits between 12V and GND
- [ ] Verify USB-C connection (D+/D-)
- [ ] Verify I2C bus (SDA/SCL pull-ups)
- [ ] Verify SPI bus (MOSI/SCK/MISO)
- [ ] Verify UART0 (TX/RX)
- [ ] Flash firmware and test

## Fabrication

### Gerber Files

Generate Gerber files from KiCad:
1. Open schematic in KiCad
2. Update PCB from schematic
3. Go to File → Plot → Gerber Files
4. Select all layers
5. Plot
6. Zip the output files

### Order from JLCPCB/PCBWay

1. Upload Gerber files
2. Select specifications:
   - Layers: 2
   - Board size: 80×60mm
   - Thickness: 1.6mm
   - Copper weight: 1oz
   - Surface finish: HASL (Lead-free)
   - Silkscreen: White
   - Solder mask: Green
3. Select assembly (optional)
4. Submit order

### Assembly Service (Optional)

JLCPCB and PCBWay offer assembly services:
1. Upload Gerber files
2. Upload BOM file
3. Upload pick-and-place file
4. Select components
5. Submit order

## Maintenance

### Battery Replacement

CR2032 battery (BAT1) should be replaced every 2-3 years:
1. Remove old battery from holder
2. Insert new CR2032 battery (positive side up)
3. Verify RTC is keeping time

### Sensor Replacement

**BME280:**
- Soldered QFN-8 package
- Replace with hot air rework station

**GT911:**
- Soldered QFN-24 package
- Replace with hot air rework station

**MQ-7:**
- External module (JST-XH connector)
- Simply unplug and replace

**DS18B20:**
- External probe (JST-XH connector)
- Simply unplug and replace

## Revision History

| Version | Date | Changes |
|---|---|---|
| 1.0 | 2024-01-01 | Initial design |

## References

- [KiCad Documentation](https://kicad.org/docs/)
- [JLCPCB Assembly Guide](https://support.jlcpcb.com/article/26-how-to-prepare-the-bom-and-place-files)
- [PCBWay Assembly Guide](https://www.pcbway.com/blog/technology/How_to_prepare_BOM_and_Pick_and_Place_files_for_PCB_assembly.html)
