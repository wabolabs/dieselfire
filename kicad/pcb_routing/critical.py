#!/usr/bin/env python3
"""Emit critical-impedance routes into Afterburner-Modern.kicad_pcb.

Pre-routes the load-bearing traces so the autorouter has a good starting point:

  - Power rails: 12V → MP2451 → 5V → AP2112 → 3.3V (wide traces for current)
  - USB D+/D- from USB-C to ESP32-S3 (moderate width, short path)
  - SPI display bus: MOSI, SCK, CS, DC, RST (standard signal width)
  - I2C bus: SDA, SCL to BME280, DS3231, GT911
  - Blue Wire UART: TX/RX to heater connector
  - 12V sense line to heater connector

This script must run AFTER pcb_placement/apply.py has placed the components.

Run from project root:
    python3 kicad/pcb_routing/critical.py
"""

from __future__ import annotations

import re
import sys
import uuid as _uuid
from dataclasses import dataclass, field
from pathlib import Path

HERE = Path(__file__).resolve().parent
PROJECT_ROOT = HERE.parents[0]
PCB_PATH = PROJECT_ROOT / "pcb" / "Afterburner-Modern.kicad_pcb"


def _u(seed: str) -> str:
    return str(_uuid.uuid5(_uuid.NAMESPACE_URL, f"dieselfire/{seed}"))


@dataclass
class Route:
    name: str
    net: str
    _coords: list[float] = field(default_factory=list)
    width: float = 0.25  # mm
    layer: str = "F.Cu"

    @property
    def points(self) -> list[tuple[float, float]]:
        coords = self._coords
        return [(coords[i], coords[i+1]) for i in range(0, len(coords), 2)]

    def add(self, x: float, y: float | None = None):
        if y is None:
            self._coords.append(x)
        else:
            self._coords.extend([x, y])
        return self


def build_routes() -> list[Route]:
    """Define all critical routes based on placed component positions.
    
    Coordinates match pcb_placement.plan.OVERRIDES (bottom-left origin, mm).
    """
    routes: list[Route] = []

    # ===================================================================
    # POWER RAILS
    # ===================================================================

    # 12V: Terminal block J2 (8, 6) → MP2451 U2 (29, 30)
    # Width: 1.0mm for high current
    routes.append(Route("12V_in", "12V", width=1.0))
    routes[-1].add(8, 6).add(18, 18).add(29, 30)

    # 12V sense: J2 (8, 6) → J3 heater (8, 18)
    routes.append(Route("12V_sense", "12V_SENSE", width=0.5))
    routes[-1].add(8, 6).add(8, 18)

    # 5V: MP2451 U2 (29, 30) → AP2112 U3 (29, 42)
    routes.append(Route("5V rail", "5V", width=0.5))
    routes[-1].add(29, 30).add(29, 42)

    # 3.3V: AP2112 U3 (29, 42) → ESP32 U1 (38, 25) area
    routes.append(Route("3V3 rail", "3V3", width=0.3))
    routes[-1].add(29, 42).add(33, 38).add(38, 30).add(38, 25)

    # ===================================================================
    # USB (Native USB on ESP32-S3)
    # ===================================================================

    # USB D+: J1 USB-C (72, 6) → U1 ESP32 (38, 25)
    routes.append(Route("USB_DP", "USB_D+", width=0.2))
    routes[-1].add(72, 6).add(65, 14).add(55, 20).add(45, 24).add(38, 25)

    # USB D-: J1 USB-C (72, 6) → U1 ESP32 (38, 25)
    routes.append(Route("USB_DM", "USB_D-", width=0.2))
    routes[-1].add(72, 6).add(65, 14).add(55, 20).add(45, 24).add(38, 25)

    # USB GND shells: J1 shield pins
    routes.append(Route("USB_GND", "GND", width=0.5))
    routes[-1].add(72, 6).add(70, 8).add(68, 6)

    # ===================================================================
    # SPI Display Bus
    # ===================================================================

    # SPI MOSI: U1 (38, 25) → U11 display (48, 38)
    routes.append(Route("SPI_MOSI", "SPI_MOSI", width=0.2))
    routes[-1].add(38, 25).add(40, 30).add(44, 34).add(48, 38)

    # SPI SCK: U1 (38, 25) → U11 display (48, 38)
    routes.append(Route("SPI_SCK", "SPI_SCK", width=0.2))
    routes[-1].add(38, 25).add(40, 30).add(43, 34).add(48, 38)

    # SPI MISO: U1 (38, 25) → U11 display (48, 38)
    routes.append(Route("SPI_MISO", "SPI_MISO", width=0.2))
    routes[-1].add(38, 25).add(42, 30).add(46, 34).add(48, 38)

    # SPI CS: U1 (38, 25) → U11 display (48, 38)
    routes.append(Route("SPI_CS", "SPI_CS", width=0.2))
    routes[-1].add(38, 25).add(40, 30).add(44, 34).add(48, 38)

    # LCD DC: U1 (38, 25) → U11 display
    routes.append(Route("LCD_DC", "LCD_DC", width=0.2))
    routes[-1].add(38, 25).add(41, 30).add(45, 34).add(48, 38)

    # LCD RST: U1 (38, 25) → U11 display
    routes.append(Route("LCD_RST", "LCD_RST", width=0.2))
    routes[-1].add(38, 25).add(43, 30).add(47, 34).add(48, 38)

    # LCD Backlight: U1 (38, 25) → U11 display
    routes.append(Route("LCD_BL", "LCD_BL", width=0.3))
    routes[-1].add(38, 25).add(45, 30).add(47, 34).add(48, 38)

    # ===================================================================
    # I2C Bus (shared: BME280 U4, DS3231 U7, GT911 U10)
    # ===================================================================

    # I2C SDA: U1 (38, 25) → U4 (62, 50), U7 (62, 40), U10 (48, 50)
    routes.append(Route("I2C_SDA", "I2C_SDA", width=0.2))
    routes[-1].add(38, 25).add(45, 28).add(55, 35).add(62, 40)
    routes[-1].add(62, 40).add(62, 50)
    routes[-1].add(62, 50).add(55, 50).add(48, 50)

    # I2C SCL: U1 (38, 25) → U4 (62, 50), U7 (62, 40), U10 (48, 50)
    routes.append(Route("I2C_SCL", "I2C_SCL", width=0.2))
    routes[-1].add(38, 25).add(42, 28).add(52, 35).add(62, 40)
    routes[-1].add(62, 40).add(62, 50)
    routes[-1].add(62, 50).add(55, 50).add(48, 50)

    # ===================================================================
    # Blue Wire UART (heater communication)
    # ===================================================================

    # UART TX: U1 (38, 25) → U9 74LCX125 (8, 44) → BSS138 U8 (8, 36) → J3 heater (8, 18)
    routes.append(Route("BLUE_TX", "BLUE_TX", width=0.2))
    routes[-1].add(38, 25).add(28, 30).add(18, 38).add(8, 44)
    routes[-1].add(8, 44).add(8, 36)
    routes[-1].add(8, 36).add(8, 18)

    # UART RX: U1 (38, 25) → BSS138 U8 (14, 36) → J3 heater (8, 18)
    routes.append(Route("BLUE_RX", "BLUE_RX", width=0.2))
    routes[-1].add(38, 25).add(28, 30).add(18, 35).add(14, 36)
    routes[-1].add(14, 36).add(10, 28).add(8, 18)

    # Tx Gate control: U1 (38, 25) → U9 74LCX125 (8, 44)
    routes.append(Route("TX_GATE", "TX_GATE", width=0.2))
    routes[-1].add(38, 25).add(28, 30).add(18, 38).add(8, 44)

    # ===================================================================
    # OneWire Temp Sensor (DS18B20)
    # ===================================================================

    # TEMP_SENSOR: U1 (38, 25) → J4 DS18B20 (36, 6)
    routes.append(Route("TEMP_SENSOR", "TEMP_SENSOR", width=0.2))
    routes[-1].add(38, 25).add(37, 18).add(36, 6)

    # CO_AOUT (ADC): U1 (38, 25) → J5 MQ-7 (48, 6)
    routes.append(Route("CO_AOUT", "CO_AOUT", width=0.2))
    routes[-1].add(38, 25).add(43, 18).add(48, 6)

    # CO_DOUT (digital): U1 (38, 25) → J5 MQ-7 (48, 6)
    routes.append(Route("CO_DOUT", "CO_DOUT", width=0.2))
    routes[-1].add(38, 25).add(45, 18).add(48, 6)

    # ===================================================================
    # Touch Controller (GT911)
    # ===================================================================

    # TOUCH_INT: U1 (38, 25) → U10 GT911 (48, 50)
    routes.append(Route("TOUCH_INT", "TOUCH_INT", width=0.2))
    routes[-1].add(38, 25).add(40, 35).add(44, 42).add(48, 50)

    # TOUCH_RST: U1 (38, 25) → U10 GT911 (48, 50)
    routes.append(Route("TOUCH_RST", "TOUCH_RST", width=0.2))
    routes[-1].add(38, 25).add(42, 35).add(46, 42).add(48, 50)

    # ===================================================================
    # Buttons & LEDs
    # ===================================================================

    # POWER_BUTTON: U1 (38, 25) → SW1 (24, 30)
    routes.append(Route("POWER_BTN", "BUTTON1", width=0.2))
    routes[-1].add(38, 25).add(32, 28).add(24, 30)

    # RESET_BTN: U1 (38, 25) → SW2 (46, 30)
    routes.append(Route("RESET_BTN", "BUTTON2", width=0.2))
    routes[-1].add(38, 25).add(42, 28).add(46, 30)

    # STATUS_LED: U1 (38, 25) → D1 (38, 30)
    routes.append(Route("STATUS_LED", "LED1", width=0.2))
    routes[-1].add(38, 25).add(38, 30)

    # CO_LED: U1 (38, 25) → D2 (38, 40)
    routes.append(Route("CO_LED", "LED2", width=0.2))
    routes[-1].add(38, 25).add(38, 32).add(38, 40)

    # ===================================================================
    # GND connections (key power ground points)
    # ===================================================================

    # GND: U1 ESP32 ground pad → GND pour reference
    routes.append(Route("GND_MCU", "GND", width=0.3))
    routes[-1].add(38, 25).add(36, 23).add(34, 25)

    return routes


def find_or_create_net(pcb_text: str, net_name: str) -> int:
    """Find existing net number for a name, or return the next available."""
    # Parse net table: (net N "name")
    net_map: dict[str, int] = {}
    for m in re.finditer(r'\(net\s+(\d+)\s+"([^"]*)"\)', pcb_text):
        net_map[m.group(2)] = int(m.group(1))
    
    # Find next available net number
    existing_nums = set(net_map.values())
    next_num = 1
    while next_num in existing_nums:
        next_num += 1
    
    if net_name in net_map:
        return net_map[net_name]
    
    return next_num


def add_nets_and_routes(pcb_path: Path) -> None:
    """Add nets to the net table and emit segment entries for critical routes."""
    text = pcb_path.read_text()
    
    routes = build_routes()
    
    # Find position of (net 1 "") in the net table section
    # We need to insert our nets before the first footprint
    first_fp = text.find('\t(footprint "')
    if first_fp == -1:
        print("ERROR: no footprints found in PCB file")
        return
    
    # Find the (net ... section - it's right before footprints
    net_section_end = text.rfind('\t(net ', 0, first_fp)
    if net_section_end == -1:
        print("ERROR: no net section found")
        return
    
    # Build net entries and segment entries
    net_entries: dict[str, int] = {}  # net_name → net_number
    segment_entries = []
    
    # Collect all unique net names from routes
    net_names = list(dict.fromkeys(r.net for r in routes))  # unique, ordered
    
    # Find existing net numbers and assign new ones
    next_net = 1
    
    # First, read existing nets
    existing_nets: dict[str, int] = {}
    for m in re.finditer(r'\(net\s+(\d+)\s+"([^"]*)"\)', text[:first_fp]):
        existing_nets[m.group(2)] = int(m.group(1))
        next_net = max(next_net, int(m.group(1)) + 1)
    
    # Assign net numbers
    for name in net_names:
        if name in existing_nets:
            net_entries[name] = existing_nets[name]
        else:
            net_entries[name] = next_net
            next_net += 1
    
    # Build net table additions (only NEW nets, not duplicates)
    new_net_lines = []
    for name, num in net_entries.items():
        if name not in existing_nets:
            new_net_lines.append(f'\t(net {num} "{name}")')
    
    # Build segment entries
    for route in routes:
        net_num = net_entries.get(route.net, existing_nets.get(route.net, 0))
        for i in range(len(route.points) - 1):
            p1 = route.points[i]
            p2 = route.points[i + 1]
            seg = (
                f'\t(segment '
                f'(start {p1[0]} {p1[1]}) '
                f'(end {p2[0]} {p2[1]}) '
                f'(width {route.width}) '
                f'(layer "{route.layer}") '
                f'(net {net_num}) '
                f'(uuid "{_u(route.name + "/" + str(i))}"))'
            )
            segment_entries.append(seg)
    
    # Insert new net entries after existing net entries
    # Find the last (net ...) line before footprints
    last_net_match = None
    for m in re.finditer(r'\(net\s+\d+\s+"[^"]*"\)', text[:first_fp]):
        last_net_match = m
    
    if last_net_match:
        insert_pos = last_net_match.end()
    else:
        # Find (net 1 "") and insert after it
        m = re.search(r'\(net\s+1\s+"[^"]*"\)', text[:first_fp])
        insert_pos = m.end() if m else first_fp
    
    # Insert net additions
    if new_net_lines:
        text = text[:insert_pos] + '\n' + '\n'.join(new_net_lines) + text[insert_pos:]
    
    # Insert segments at the top level, just before the final closing paren
    # of the kicad_pcb file. We need to find the position right before the
    # last ')' that closes the main (kicad_pcb ...) block.
    # Count depth from the start to find the top level.
    depth = 0
    seg_start = len(text) - 1
    # Walk backwards from the end to find the position before the final ')'
    for i in range(len(text) - 1, -1, -1):
        if text[i] == ')':
            depth -= 1
            if depth == 0:
                seg_start = i
                break
        elif text[i] == '(':
            depth += 1
    
    # Insert segments
    if segment_entries:
        seg_text = '\n' + '\n'.join(segment_entries) + '\n'
        text = text[:seg_start] + seg_text + text[seg_start:]
    
    pcb_path.write_text(text)
    
    total_segments = sum(len(r.points) - 1 for r in routes)
    print(f"Added {len(net_entries)} nets and {total_segments} route segments")
    print(f"Net names: {', '.join(net_entries)}")


if __name__ == "__main__":
    add_nets_and_routes(PCB_PATH)
    print(f"\nCritical routes added to {PCB_PATH}")
    print("Run pcb_placement/apply.py first if you haven't placed components yet.")
