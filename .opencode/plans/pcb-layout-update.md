# PCB Layout Update - Plan

## Overview
Update `plan.py` OVERRIDES to match the manually-adjusted test file layout, which has better spacing and component distribution. Then re-run the placement and autorouting pipeline.

## Changes to `plan.py` (OVERRIDES)

### Moved Components (from plan to test positions)

| Component | Old Position | New Position | Reason |
|-----------|-------------|-------------|--------|
| U1 (ESP32) | (38, 28) | (37, 19) | Moved down for better spacing |
| U11 (Display) | (40, 48) | (48, 39) | Moved right/down, closer to board center |
| U2 (MP2451) | (32, 34) | (29, 30) | Moved down-left |
| U7 (DS3231) | (70, 50) | (53, 21) | Moved from top-right to center |
| BAT1 (CR2032) | (70, 50) | (19, 54) | Moved from top-right to top-left |
| J1 (USB-C) | (72, 6) | (74, 7) | Slight adjustment |
| J2 (Terminal) | (8, 6) | (9, 22) | Moved up along edge |
| J3 (Heater) | (8, 14) | (7, 30) | Moved up |
| J4 (DS18B20) | (66, 6) | (36, 6) | Moved to bottom-center, clear C9 |
| J5 (MQ-7) | (74, 6) | (47, 8) | Moved left to bottom-center |
| H1 (Header 1) | (66, 16) | (72, 18) | Adjusted |
| H2 (Header 2) | (66, 30) | (62, 18) | Adjusted |
| U8_1 (BSS138 TX) | (8, 44) | (8.4, 40) | Adjusted |
| U8_2 (BSS138 RX) | (12, 44) | (14.4, 40) | Adjusted |
| SW1 (Power) | (34, 34) | (10, 23) | **Left edge** - power button |
| SW2 (Reset) | (46, 34) | (16, 23) | **Left edge** - reset button |

### New Overrides (added to OVERRIDES)

| Component | Position | Reason |
|-----------|----------|--------|
| C10 | (43, 10) | Moved from decoup zone to clear J4 |

### Updated Exclusion Sizes

| Component | Old Half-Size | New Half-Size | Reason |
|-----------|--------------|---------------|--------|
| U1 | (4.0, 6.0) | (4.0, 6.0) | No change |
| U11 | (8.0, 6.0) | (8.0, 6.0) | No change |
| U7 | (3.0, 3.0) | (3.0, 3.0) | No change |
| BAT1 | (4.0, 4.0) | (4.0, 4.0) | No change |

## Changes to `apply.py` (zone_map)

Remove C10 from automatic zone placement:
```python
# In determine_zone(), add:
"C10": "decoup",  # explicit zone assignment
```

## Changes to `critical.py` (BUTTON routes)

Update BUTTON1 and BUTTON2 route coordinates to match new switch positions:

```python
# BUTTON1: U1 (37, 19) → SW1 (10, 23)
routes.append(Route("POWER_BTN", "BUTTON1", width=0.2))
routes[-1].add(37, 19).add(25, 22).add(10, 23)

# BUTTON2: U1 (37, 19) → SW2 (16, 23)
routes.append(Route("RESET_BTN", "BUTTON2", width=0.2))
routes[-1].add(37, 19).add(28, 22).add(16, 23)
```

## Execution Order

1. Copy test file → working PCB:
   ```bash
   cp kicad/pcb/Afterburner-Modern-test.kicad_pcb kicad/pcb/Afterburner-Modern.kicad_pcb
   cp kicad/pcb/Afterburner-Modern-test.kicad_prl kicad/pcb/Afterburner-Modern.kicad_prl
   cp kicad/pcb/Afterburner-Modern-test.kicad_pro kicad/pcb/Afterburner-Modern.kicad_pro
   ```

2. Update plan.py with new positions (create new OVERRIDES)

3. Update apply.py zone_map for C10

4. Update critical.py BUTTON routes

5. Run pipeline:
   ```bash
   python3 kicad/pcb_placement/apply.py
   python3 kicad/pcb_routing/critical.py
   python3 kicad/pcb_routing/autoroute.py
   ```

## Verification Checklist

- [ ] All 46 footprints placed on-board (no off-board components)
- [ ] No overlaps between components (< 3mm clearance)
- [ ] J4 (36, 6) has > 2mm clearance from C9 (34, 10)
- [ ] C10 (43, 10) has > 2mm clearance from J4 and J5
- [ ] SW1 (10, 23) and SW2 (16, 23) are on left edge with 6mm spacing
- [ ] DRC passes with 0 violations
- [ ] All nets connected (or connected via GND pours)
- [ ] Fabrication files exported

## Expected Route Lengths

| Route | Plan Length | Test Length | Δ |
|-------|-------------|-------------|---|
| 12V: J2→U2 | 36.9mm | 21.6mm | -15.3mm ✓ |
| 5V: U2→U3 | 8.0mm | 12.4mm | +4.4mm |
| 3V3: U3→U1 | 15.2mm | 23.6mm | +8.3mm |
| USB: J1→U1 | 40.5mm | 38.7mm | -1.8mm ✓ |
| SPI: U1→U11 | 20.1mm | 22.3mm | +2.2mm |
| I2C: U1→U10 | 26.8mm | 35.4mm | +8.6mm |
| BlueTX: U1→J3 | 33.1mm | 32.0mm | -1.1mm ✓ |
| TEMP: U1→J4 | 35.6mm | 11.2mm | -24.4mm ✓✓ |
| CO: U1→J5 | 42.2mm | 14.6mm | -27.6mm ✓✓ |
| BUTTON1: U1→SW1 | 19.6mm | ~28mm | (new) |
| BUTTON2: U1→SW2 | 19.6mm | ~23mm | (new) |
