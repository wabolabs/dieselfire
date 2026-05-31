#!/usr/bin/env python3
"""Placement plan for DieselFire S3 — zone-based layout with proper spacing.

Board: 80mm x 60mm, origin at bottom-left (KiCad PCB convention).
Mounting holes at (4,4), (76,4), (4,56), (76,56).

Layout strategy: 7 functional zones with ICs placed first (overrides),
then passives tiled near their ICs in dedicated zones.

Each component is placed via one of three strategies (in priority order):

1. **OVERRIDES**: ref → (x, y, rot, layer). Use for connectors, ICs, and any
   part whose exact position matters.
2. **SHEET_ZONES**: functional zone → rectangular tile zone (x, y, w, h).
   Components without an override get tiled into the zone.
3. **DEFAULT_PARK**: components with no zone match get parked off-board.

Coordinates are in mm. Board outline: 0,0 to 80,60 (bottom-left origin).
"""

from __future__ import annotations

from dataclasses import dataclass


@dataclass
class Zone:
    x: float
    y: float
    w: float
    h: float
    pitch: float = 3.0
    cols: int = 0


# Functional zones for tiling passives near their ICs.
# Zones sized to hold all passives with 3mm pitch and clearance.
SHEET_ZONES: dict[str, Zone] = {
    # Power passives: resistors near U2/U3 (power regulation)
    "power_r":        Zone(x=22,  y=20,  w=18, h=12),  # near U2 (29,30)
    # MCU decoupling caps: near U1 (38,25)
    "mcu_c":          Zone(x=30,  y=14,  w=25, h=12),  # around U1
    # Sensor passives: near U4/U7 (top-right sensor cluster)
    "sensor_c":       Zone(x=54,  y=42,  w=20, h=12),  # near U4, U7
    # I/O passives: near U9/U8 (left edge I/O buffers)
    "io_c":           Zone(x=4,   y=30,  w=20, h=12),  # near U9, U8_1, U8_2
}


# Component-specific overrides. Format: ref → (x, y, rotation_degrees, layer).
OVERRIDES: dict[str, tuple[float, float, float, str]] = {
    # ---- Power input (bottom-left corner) ----------------------------------
    "J2":            (8,    6,   90,  "B.Cu"),   # 2-pin terminal block (12V IN)

    # ---- USB-C (bottom-right corner) ---------------------------------------
    "J1":            (72,   6,   90,  "B.Cu"),   # USB-C connector

    # ---- Power regulation (bottom-center, near U2) -------------------------
    "U2":            (29,   30,  0,   "F.Cu"),   # MP2451 buck (12V→5V)
    "U3":            (29,   42,  0,   "F.Cu"),   # AP2112 LDO (5V→3.3V)

    # ---- ESP32-S3 module (center) ------------------------------------------
    "U1":            (38,   25,  0,   "F.Cu"),   # ESP32-S3-WROOM-1-N8R8

    # ---- Buttons (flanking U1, spaced from U2) -----------------------------
    "SW1":           (24,   30,  0,   "F.Cu"),   # Power button (left of U2)
    "SW2":           (46,   30,  0,   "F.Cu"),   # Reset button (right of U1)

    # ---- LEDs (near U1) ----------------------------------------------------
    "D1":            (38,   30,  0,   "F.Cu"),   # Status LED (green)
    "D2":            (38,   40,  0,   "F.Cu"),   # CO alarm LED (red)

    # ---- Display + Touch (top-center, adjacent for FPC cable) --------------
    "U11":           (48,   38,  0,   "F.Cu"),   # ILI9341 TFT (FPC top)
    "U10":           (48,   50,  0,   "F.Cu"),   # GT911 touch controller

    # ---- Sensors (top-right, grouped for I2C) ------------------------------
    "U4":            (62,   50,  0,   "F.Cu"),   # BME280
    "U7":            (62,   40,  0,   "F.Cu"),   # DS3231 RTC
    "BAT1":          (74,   50,  0,   "F.Cu"),   # CR2032 holder

    # ---- I/O buffers (left edge, near J3) ----------------------------------
    "U9":            (8,    44,  0,   "F.Cu"),   # 74LCX125 Tx buffer
    "U8_1":          (8,    36,  0,   "F.Cu"),   # BSS138 level shifter (TX)
    "U8_2":          (14,   36,  0,   "F.Cu"),   # BSS138 level shifter (RX)

    # ---- Heater connector (left edge) --------------------------------------
    "J3":            (8,    18,  0,   "F.Cu"),   # JST-XH 3-pin blue wire

    # ---- External sensors (bottom-center) ----------------------------------
    "J4":            (36,   6,   0,   "F.Cu"),   # JST-XH 3-pin DS18B20
    "J5":            (48,   6,   0,   "F.Cu"),   # JST-XH 4-pin MQ-7

    # ---- Expansion headers (right edge) ------------------------------------
    "H1":            (70,   14,  0,   "F.Cu"),   # 2x10 expansion header 1
    "H2":            (70,   26,  0,   "F.Cu"),   # 2x10 expansion header 2

    # ---- MCU decoupling caps (explicit placement) --------------------------
    "C1":            (33,   14,  0,   "F.Cu"),   # near U1 pin 1

    # ---- Mounting holes ----------------------------------------------------
    "MH1":           (4,    4,   0,   "F.Cu"),   # bottom-left
    "MH2":           (76,   4,   0,   "F.Cu"),   # bottom-right
    "MH3":           (4,    56,  0,   "F.Cu"),   # top-left
    "MH4":           (76,   56,  0,   "F.Cu"),   # top-right
}

# Off-board parking zone for unplaced components.
DEFAULT_PARK = Zone(x=100, y=0, w=80, h=60)


def _exclusion_halfsize(ref: str) -> tuple[float, float]:
    """Half-width / half-height of an override's exclusion box."""
    if ref == "U1":
        return (6.0, 8.0)   # ESP32-S3 module (18×24 footprint)
    if ref in ("H1", "H2"):
        return (12.0, 2.0)  # 2x10 header — long, narrow
    if ref in ("J1", "J2"):
        return (3.0, 2.0)   # USB-C + terminal block
    if ref == "U11":
        return (10.0, 8.0)  # 2.8" TFT display
    if ref.startswith("U10"):
        return (4.0, 4.0)   # GT911 QFN
    if ref.startswith("U2") or ref.startswith("U3"):
        return (4.0, 4.0)   # SOT-23-6 / SOT-89-3
    if ref.startswith(("U4", "U7")):
        return (3.0, 3.0)   # QFN-8 / SOIC-16
    if ref.startswith("U8") or ref.startswith("U9"):
        return (3.0, 3.0)   # SOT-23 / SOIC-14
    if ref.startswith("J3") or ref.startswith("J4") or ref.startswith("J5"):
        return (4.0, 3.0)   # JST-XH connectors
    if ref.startswith(("SW", "D")):
        return (2.0, 2.0)   # buttons + LEDs
    if ref.startswith("BAT"):
        return (4.0, 4.0)   # CR2032 holder
    if ref.startswith("U"):
        return (3.0, 3.0)   # generic IC
    return (2.0, 2.0)       # small passives


_EXCLUSIONS: list[tuple[float, float, float, float]] | None = None


def _build_exclusions() -> list[tuple[float, float, float, float]]:
    """Return list of (xmin, ymin, xmax, ymax) keep-out boxes from OVERRIDES."""
    boxes: list[tuple[float, float, float, float]] = []
    for ref, (x, y, _, _) in OVERRIDES.items():
        hw, hh = _exclusion_halfsize(ref)
        boxes.append((x - hw, y - hh, x + hw, y + hh))
    return boxes


def _blocked(x: float, y: float) -> bool:
    global _EXCLUSIONS
    if _EXCLUSIONS is None:
        _EXCLUSIONS = _build_exclusions()
    for xmin, ymin, xmax, ymax in _EXCLUSIONS:
        if xmin <= x <= xmax and ymin <= y <= ymax:
            return True
    return False


def get_placement(
    ref: str,
    zone_name: str | None,
    zone_index: int,
) -> tuple[float, float, float, str]:
    """Return (x, y, rotation, layer) for a single component.

    Tiles passives within their zone, skipping grid cells blocked by
    override exclusion boxes.
    """
    if ref in OVERRIDES:
        return OVERRIDES[ref]
    zone = SHEET_ZONES.get(zone_name or "", DEFAULT_PARK)
    cols = zone.cols or max(1, int(zone.w // zone.pitch))
    rows = max(1, int(zone.h // zone.pitch))

    slot = 0
    for row in range(rows + 2):
        for col in range(cols):
            x = zone.x + col * zone.pitch
            y = zone.y + row * zone.pitch
            if _blocked(x, y):
                continue
            if slot == zone_index:
                return (x, y, 0.0, "F.Cu")
            slot += 1

    parked_col = zone_index % max(1, int(DEFAULT_PARK.w // DEFAULT_PARK.pitch))
    parked_row = zone_index // max(1, int(DEFAULT_PARK.w // DEFAULT_PARK.pitch))
    return (DEFAULT_PARK.x + parked_col * DEFAULT_PARK.pitch,
            DEFAULT_PARK.y + parked_row * DEFAULT_PARK.pitch,
            0.0, "F.Cu")
