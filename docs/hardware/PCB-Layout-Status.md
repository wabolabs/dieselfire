# DieselFire S3 - PCB Layout Status

## Current State

The PCB layout has been generated using KiCad's Python API with the following elements:

### Completed
- **Board outline**: 80mm x 60mm with rounded corners
- **Mounting holes**: 4x M3 (3.2mm drill) at corners
- **Copper zones**: 
  - GND plane on both layers
  - 3.3V, 5V, 12V power zones on top layer
- **Ground vias**: Grid of vias connecting top and bottom ground planes
- **Test points**: TP_GND, TP_3V3, TP_5V, TP_12V
- **Silkscreen**: Board name and revision area

### Needs Manual Completion
- **Component footprints**: Need to be placed and routed in KiCad GUI
- **Trace routing**: Manual routing required between components
- **Component placement**: Optimize layout for signal integrity

## How to Complete the PCB Layout

### 1. Open in KiCad
```bash
kicad pcb/Afterburner-Modern.kicad_pcb
```

### 2. Add Component Footprints
Use the footprint library (`footprints/Afterburner.kicad_fp`) or KiCad's built-in libraries:

| Reference | Component | Footprint | Position (mm) |
|---|---|---|---|
| J1 | USB-C | USB-C-S6 | (-30, 25) |
| J2 | Terminal Block | TerminalBlock_2Pin | (-30, -25) |
| J3 | Blue Wire JST-XH | JST-XH-3 | (30, 20) |
| J4 | DS18B20 JST-XH | JST-XH-3 | (30, -20) |
| J5 | MQ-7 JST-XH | JST-XH-4 | (30, -25) |
| U1 | ESP32-S3 | ESP32_S3_WROOM | (0, 0) |
| U2 | MP2451 | SOT-23-6 | (-20, -20) |
| U3 | AP2112 | SOT-89-3 | (-15, -20) |
| U4 | BME280 | QFN-8_2x2 | (-25, 10) |
| U5 | DS3231 | SOIC-16 | (-25, -10) |
| U6 | GT911 | QFN-24_4x4 | (20, 15) |
| U7, U8 | BSS138 | SOT-23 | (-20, 15), (-20, 10) |
| U9 | 74LCX125 | SOIC-14 | (-25, 20) |
| H1, H2 | Expansion Headers | HDR_2X10 | (-30, -15), (-25, -15) |
| Y1 | Crystal | HC-49S | (-20, -25) |
| R1-R8 | Resistors | 0402 | Various |
| C1-C8 | Capacitors | 0402 | Various |
| D1, D2 | LEDs | 0603 | (-28, -10), (-26, -10) |
| SW1, SW2 | Buttons | 6x6mm | (-28, -15), (-26, -15) |

### 3. Route Traces
Follow the design document for trace routing:
- **12V traces**: ≥1mm width
- **5V traces**: ≥0.5mm width
- **3.3V traces**: ≥0.3mm width
- **Signal traces**: ≥0.2mm width
- **SPI traces**: Keep short (<50mm)
- **UART traces**: Shield from noise

### 4. Run DRC
After routing, run Design Rules Check:
```bash
kicad-cli pcb drc pcb/Afterburner-Modern.kicad_pcb
```

## Design Reference

See `docs/design/DieselFire-S3-Design.md` for:
- Complete schematic
- Pin mappings
- Power budget
- Communication interfaces

## Next Steps

1. Open PCB in KiCad GUI
2. Place all component footprints
3. Route traces according to design rules
4. Run DRC and fix violations
5. Generate Gerber files for fabrication
6. Order PCB from manufacturer (JLCPCB/PCBWay)
