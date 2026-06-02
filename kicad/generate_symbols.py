#!/usr/bin/env python3
"""
Generate KiCad 9.0 schematic symbol library for DieselFire.
Uses dataclasses to define symbols, then serializes to KiCad text format.
"""

from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional

BASE = Path(__file__).parent


@dataclass
class Effect:
    font_size: str = "0.8 0.8"
    justify: str = ""

    def serialize(self):
        parts = [f"(size {self.font_size})"]
        if self.justify:
            parts.append(f"(justify {self.justify})")
        return f"(effects (font {parts[0]} {' '.join(parts[1:])}))"


@dataclass
class Text:
    content: str
    at_x: float = 0
    at_y: float = 0
    rotation: float = 0
    font_size: str = "0.8 0.8"
    justify: str = ""

    def serialize(self):
        eff = f"(size {self.font_size})"
        if self.justify:
            eff += f" (justify {self.justify})"
        return f"(text \"{self.content}\" (at {self.at_x} {self.at_y} {self.rotation})\n  (effects {eff}))"


@dataclass
class Property:
    name: str
    value: str
    id: int
    at_x: float = 0
    at_y: float = 0
    rotation: float = 0
    font_size: str = "0.8 0.8"
    justify: str = "left"

    def serialize(self):
        eff = f"(size {self.font_size}) (justify {self.justify})"
        return (f'(property "{self.name}" "{self.value}" (id {self.id}) '
                f"(at {self.at_x} {self.at_y} {self.rotation})\n"
                f"  (effects (font {eff})))")


@dataclass
class Pin:
    name: str
    type: str = "push_thru"  # push_thru, input, output, power_in, passive_linear, etc.
    at_x: float = 0
    at_y: float = 0
    rotation: int = 0
    length: float = 5.08
    uuid: str = ""

    def serialize(self):
        shape = "linear"
        return (f'(pin {self.type} {shape} (at {self.at_x} {self.at_y} {self.rotation}) '
                f"(length {self.length})\n"
                f'  (name "{self.name}" (effects (font (size 0.8 0.8))))\n'
                f'  (uuid "{self.uuid}"))')


@dataclass
class Rect:
    start_x: float
    start_y: float
    end_x: float
    end_y: float
    width: float = 0.254

    def serialize(self):
        return (f'(rect (start {self.start_x} {self.start_y}) (end {self.end_x} {self.end_y})\n'
                f'  (width {self.width}) (fill (color 0 0 0 0)))')


@dataclass
class SymbolBody:
    rect: Rect
    pins: list = field(default_factory=list)

    def serialize(self):
        lines = [self.rect.serialize()]
        for pin in self.pins:
            lines.append(pin.serialize())
        return "\n".join(lines)


@dataclass
class Symbol:
    name: str
    in_bom: bool = True
    on_board: bool = True
    properties: list = field(default_factory=list)
    body: SymbolBody = None

    def serialize(self):
        bom = "yes" if self.in_bom else "no"
        board = "yes" if self.on_board else "no"
        lines = [
            f'(symbol "{self.name}" (in_bom {bom}) (on_board {board})',
        ]
        for prop in self.properties:
            lines.append(prop.serialize())
        lines.append("  (symbol \"" + self.name + "_1\"")
        lines.append("    " + self.body.serialize())
        lines.append("  )")
        lines.append(")")
        return "\n".join(lines)


def make_uuid(n):
    """Generate a deterministic UUID from an integer."""
    hex_str = format(n, '032x')
    return f"{hex_str[:8]}-{hex_str[8:12]}-{hex_str[12:16]}-{hex_str[16:20]}-{hex_str[20:32]}"


def build_esp32_pins():
    """Build ESP32-S3 WROOM pin definitions."""
    pins = []
    # Left side pins (pin number from bottom to top)
    left_pins = [
        ("GPIO43", -17.78), ("GPIO44", -15.24), ("GPIO45", -12.7),
        ("GPIO46", -10.16), ("GPIO47", -7.62), ("GND", -5.08),
        ("3P3", -2.54), ("3P3", 0), ("GND", 2.54), ("GND", 5.08),
        ("GPIO0", 7.62), ("GPIO48", 10.16), ("GPIO49", 12.7), ("GPIO50", 15.24),
    ]
    for i, (name, y) in enumerate(left_pins):
        pins.append(Pin(name, "push_thru", -27.94, y, 0, uuid=make_uuid(1000 + i)))

    # Right side pins
    right_pins = [
        ("GPIO16", -17.78), ("GPIO17", -15.24), ("GPIO18", -12.7),
        ("GPIO19", -10.16), ("GPIO20", -7.62), ("GND", -5.08),
        ("GND", -2.54), ("GND", 0), ("3P3", 2.54), ("3P3", 5.08),
        ("GND", 7.62), ("GND", 10.16), ("GPIO12", 12.7), ("GPIO11", 15.24),
    ]
    for i, (name, y) in enumerate(right_pins):
        pins.append(Pin(name, "push_thru", 27.94, y, 2, uuid=make_uuid(1050 + i)))

    # Top/bottom power pins
    pins.append(Pin("EN", "push_thru", 0, -22.86, 1, uuid=make_uuid(1100)))
    pins.append(Pin("VDD_3P3", "power_in", 0, 22.86, 3, uuid=make_uuid(1101)))
    pins.append(Pin("GND", "power_in", -30.48, 0, 0, uuid=make_uuid(1102)))
    pins.append(Pin("VDD_3P3", "power_in", 30.48, 0, 2, uuid=make_uuid(1103)))
    return pins


def build_buck_pins():
    """Build MP2451 buck converter pins."""
    return [
        Pin("VIN", "push_thru", -10.16, 2.54, 0, uuid=make_uuid(2000)),
        Pin("GND", "push_thru", -10.16, 0, 0, uuid=make_uuid(2001)),
        Pin("SW", "push_thru", -10.16, -2.54, 0, uuid=make_uuid(2002)),
        Pin("FB", "push_thru", 10.16, -2.54, 2, uuid=make_uuid(2003)),
        Pin("EN", "push_thru", 10.16, 0, 2, uuid=make_uuid(2004)),
        Pin("NC", "push_thru", 10.16, 2.54, 2, uuid=make_uuid(2005)),
    ]


def build_ldo_pins():
    """Build AP2112 LDO pins."""
    return [
        Pin("IN", "push_thru", -7.62, 1.27, 0, uuid=make_uuid(3000)),
        Pin("GND", "push_thru", -7.62, -1.27, 0, uuid=make_uuid(3001)),
        Pin("NC", "push_thru", -7.62, -3.81, 0, uuid=make_uuid(3002)),
        Pin("NC", "push_thru", 7.62, -3.81, 2, uuid=make_uuid(3003)),
        Pin("OUT", "push_thru", 7.62, -1.27, 2, uuid=make_uuid(3004)),
    ]


def build_sensor_pins():
    """Build BME280 I2C sensor pins."""
    return [
        Pin("SDI", "push_thru", -7.62, -2.54, 0, uuid=make_uuid(4000)),
        Pin("SCK", "push_thru", -7.62, 0, 0, uuid=make_uuid(4001)),
        Pin("SDO", "push_thru", -7.62, 2.54, 0, uuid=make_uuid(4002)),
        Pin("VDD", "push_thru", 7.62, 2.54, 2, uuid=make_uuid(4003)),
        Pin("GND", "push_thru", 7.62, 0, 2, uuid=make_uuid(4004)),
        Pin("GND", "push_thru", 7.62, -2.54, 2, uuid=make_uuid(4005)),
    ]


def build_touch_pins():
    """Build GT911 touch controller pins."""
    return [
        Pin("SDA", "push_thru", -7.62, -2.54, 0, uuid=make_uuid(5000)),
        Pin("SCL", "push_thru", -7.62, 0, 0, uuid=make_uuid(5001)),
        Pin("VDD", "push_thru", 7.62, 2.54, 2, uuid=make_uuid(5002)),
        Pin("GND", "push_thru", 7.62, 0, 2, uuid=make_uuid(5003)),
        Pin("INT", "push_thru", 7.62, -2.54, 2, uuid=make_uuid(5004)),
        Pin("RST", "push_thru", -7.62, 2.54, 0, uuid=make_uuid(5005)),
    ]


def build_rtc_pins():
    """Build DS3231 RTC pins."""
    return [
        Pin("VCC", "push_thru", -10.16, -2.54, 0, uuid=make_uuid(6000)),
        Pin("GND", "push_thru", -10.16, 0, 0, uuid=make_uuid(6001)),
        Pin("SCL", "push_thru", -10.16, 2.54, 0, uuid=make_uuid(6002)),
        Pin("SDA", "push_thru", -10.16, 5.08, 0, uuid=make_uuid(6003)),
        Pin("32K", "push_thru", 10.16, 5.08, 2, uuid=make_uuid(6004)),
        Pin("INT/", "push_thru", 10.16, 2.54, 2, uuid=make_uuid(6005)),
        Pin("X1", "push_thru", 10.16, -2.54, 2, uuid=make_uuid(6006)),
        Pin("X2", "push_thru", 10.16, -5.08, 2, uuid=make_uuid(6007)),
    ]


def build_mq7_pins():
    """Build MQ-7 CO sensor pins."""
    return [
        Pin("HEATER+", "push_thru", -7.62, 2.54, 0, uuid=make_uuid(7000)),
        Pin("HEATER-", "push_thru", -7.62, -2.54, 0, uuid=make_uuid(7001)),
        Pin("AO", "push_thru", 7.62, -2.54, 2, uuid=make_uuid(7002)),
        Pin("VO", "push_thru", 7.62, 2.54, 2, uuid=make_uuid(7003)),
    ]


def build_ds18b20_pins():
    """Build DS18B20 temperature sensor pins."""
    return [
        Pin("DQ", "push_thru", -7.62, 2.54, 0, uuid=make_uuid(8000)),
        Pin("GND", "push_thru", -7.62, -2.54, 0, uuid=make_uuid(8001)),
        Pin("VDD", "power_in", 7.62, 0, 2, uuid=make_uuid(8002)),
    ]


def build_levelshifter_pins():
    """Build BSS138 level shifter MOSFET pins."""
    return [
        Pin("G", "push_thru", -7.62, 2.54, 0, uuid=make_uuid(9000)),
        Pin("S", "push_thru", -7.62, -2.54, 0, uuid=make_uuid(9001)),
        Pin("D", "passive_linear", 7.62, 0, 2, uuid=make_uuid(9002)),
    ]


def build_usb_pins():
    """Build USB-C connector pins."""
    left = [
        ("CC1", 5.08), ("CC2", 2.54), ("TX+", 0),
        ("TX-", -2.54), ("VBUS", -5.08), ("GND", -7.62),
    ]
    right = [
        ("GND", -7.62), ("VBUS", -5.08), ("RX-", -2.54),
        ("RX+", 0), ("SBU1", 2.54), ("SBU2", 5.08),
    ]
    pins = []
    for i, (name, y) in enumerate(left):
        pins.append(Pin(name, "pass_thru", -12.7, y, 0, uuid=make_uuid(10000 + i)))
    for i, (name, y) in enumerate(right):
        pins.append(Pin(name, "pass_thru", 12.7, y, 2, uuid=make_uuid(10010 + i)))
    pins.append(Pin("SHIELD", "pass_thru", 0, 10.16, 1, uuid=make_uuid(10020)))
    return pins


def build_terminal_block_pins(count):
    """Build terminal block connector pins."""
    return [
        Pin(str(i + 1), "pass_thru", -7.62, (i - (count - 1) / 2) * 2.54, 0,
            uuid=make_uuid(11000 + i))
        for i in range(count)
    ]


def build_jst_pins(count):
    """Build JST-XH connector pins."""
    spacing = 2.54
    offset = (count - 1) * spacing / 2
    return [
        Pin(str(i + 1), "pass_thru", -7.62, offset - i * spacing, 0,
            uuid=make_uuid(12000 + i))
        for i in range(count)
    ]


def build_expansion_pins():
    """Build 2x10 expansion header pins."""
    pins = []
    # Left column (pins 1-10, bottom to top)
    for i in range(10):
        y = -17.78 + i * 2.54
        pins.append(Pin(str(i + 1), "pass_thru", -15.24, y, 0, uuid=make_uuid(13000 + i)))
    # Right column (pins 11-20, top to bottom)
    for i in range(10):
        y = 5.08 - i * 2.54
        pins.append(Pin(str(i + 11), "pass_thru", 15.24, y, 2, uuid=make_uuid(13010 + i)))
    return pins


def build_basic_components():
    """Build simple passive component symbols."""
    comps = []

    # Resistor
    comps.append(Symbol("RES", properties=[
        Property("Reference", "R", 0),
        Property("Value", "10k", 1, at_y=-2.54),
    ], body=SymbolBody(
        rect=Rect(-5.08, -2.54, 5.08, 2.54),
        pins=[
            Pin("1", "passive_linear", -7.62, 0, 0, uuid=make_uuid(14000)),
            Pin("2", "passive_linear", 7.62, 0, 2, uuid=make_uuid(14001)),
        ]
    )))

    # Capacitor
    comps.append(Symbol("CAP", properties=[
        Property("Reference", "C", 0),
        Property("Value", "10uF", 1, at_y=-2.54),
    ], body=SymbolBody(
        rect=Rect(-5.08, -2.54, 5.08, 2.54),
        pins=[
            Pin("1", "passive_linear", 0, -5.08, 3, uuid=make_uuid(15000)),
            Pin("2", "passive_linear", 0, 5.08, 1, uuid=make_uuid(15001)),
        ]
    )))

    # Inductor
    comps.append(Symbol("INDUCTOR", properties=[
        Property("Reference", "L", 0),
        Property("Value", "10uH", 1, at_y=-2.54),
    ], body=SymbolBody(
        rect=Rect(-5.08, -2.54, 5.08, 2.54),
        pins=[
            Pin("1", "passive_linear", -7.62, 0, 0, uuid=make_uuid(16000)),
            Pin("2", "passive_linear", 7.62, 0, 2, uuid=make_uuid(16001)),
        ]
    )))

    # Diode
    comps.append(Symbol("DIODE", properties=[
        Property("Reference", "D", 0),
        Property("Value", "BAT54S", 1, at_y=-2.54),
    ], body=SymbolBody(
        rect=Rect(-5.08, -2.54, 5.08, 2.54),
        pins=[
            Pin("A", "passive_linear", -7.62, 0, 0, uuid=make_uuid(17000)),
            Pin("K", "passive_linear", 7.62, 0, 2, uuid=make_uuid(17001)),
        ]
    )))

    # Crystal
    comps.append(Symbol("CRYSTAL_2PIN", properties=[
        Property("Reference", "Y", 0),
        Property("Value", "26MHz", 1, at_y=-2.54),
    ], body=SymbolBody(
        rect=Rect(-5.08, -2.54, 5.08, 2.54),
        pins=[
            Pin("1", "passive_linear", -7.62, 0, 0, uuid=make_uuid(18000)),
            Pin("2", "passive_linear", 7.62, 0, 2, uuid=make_uuid(18001)),
        ]
    )))

    # LED
    comps.append(Symbol("LED", properties=[
        Property("Reference", "D", 0),
        Property("Value", "LED", 1, at_y=-2.54),
    ], body=SymbolBody(
        rect=Rect(-5.08, -2.54, 5.08, 2.54),
        pins=[
            Pin("A", "passive_linear", -7.62, 0, 0, uuid=make_uuid(19000)),
            Pin("K", "passive_linear", 7.62, 0, 2, uuid=make_uuid(19001)),
        ]
    )))

    return comps


def build_symbols():
    """Assemble all symbols into a list."""
    symbols = []

    # ESP32-S3 WROOM
    symbols.append(Symbol("ESP32-S3-WROOM-1-N8R8", properties=[
        Property("Reference", "U", 0),
        Property("Value", "ESP32-S3-WROOM-1-N8R8", 1, at_y=-2.54),
        Property("Footprint", "ESP32_S3_WROOM", 2, at_y=5.08),
        Property("Datasheet", "", 3, at_y=7.62),
    ], body=SymbolBody(
        rect=Rect(-25.4, -20.32, 25.4, 20.32),
        pins=build_esp32_pins(),
    )))

    # MP2451 Buck
    symbols.append(Symbol("MP2451", properties=[
        Property("Reference", "U", 0),
        Property("Value", "MP2451", 1, at_y=-2.54),
        Property("Footprint", "SOT-23-6", 2, at_y=5.08),
        Property("Datasheet", "MP2451.pdf", 3, at_y=7.62),
    ], body=SymbolBody(
        rect=Rect(-7.62, -5.08, 7.62, 5.08),
        pins=build_buck_pins(),
    )))

    # AP2112 LDO
    symbols.append(Symbol("AP2112", properties=[
        Property("Reference", "U", 0),
        Property("Value", "AP2112", 1, at_y=-2.54),
        Property("Footprint", "SOT-23-5", 2, at_y=5.08),
        Property("Datasheet", "AP2112.pdf", 3, at_y=7.62),
    ], body=SymbolBody(
        rect=Rect(-5.08, -3.81, 5.08, 3.81),
        pins=build_ldo_pins(),
    )))

    # BME280
    symbols.append(Symbol("BME280", properties=[
        Property("Reference", "U", 0),
        Property("Value", "BME280", 1, at_y=-2.54),
        Property("Footprint", "QFN-24_4x4mm", 2, at_y=5.08),
        Property("Datasheet", "BME280.pdf", 3, at_y=7.62),
    ], body=SymbolBody(
        rect=Rect(-5.08, -5.08, 5.08, 5.08),
        pins=build_sensor_pins(),
    )))

    # GT911
    symbols.append(Symbol("GT911", properties=[
        Property("Reference", "U", 0),
        Property("Value", "GT911", 1, at_y=-2.54),
        Property("Footprint", "QFN-24_4x4mm", 2, at_y=5.08),
        Property("Datasheet", "GT911.pdf", 3, at_y=7.62),
    ], body=SymbolBody(
        rect=Rect(-5.08, -5.08, 5.08, 5.08),
        pins=build_touch_pins(),
    )))

    # DS3231
    symbols.append(Symbol("DS3231", properties=[
        Property("Reference", "U", 0),
        Property("Value", "DS3231", 1, at_y=-2.54),
        Property("Footprint", "SOIC-16", 2, at_y=5.08),
        Property("Datasheet", "DS3231.pdf", 3, at_y=7.62),
    ], body=SymbolBody(
        rect=Rect(-7.62, -5.08, 7.62, 5.08),
        pins=build_rtc_pins(),
    )))

    # MQ-7
    symbols.append(Symbol("MQ-7", properties=[
        Property("Reference", "R", 0),
        Property("Value", "MQ-7", 1, at_y=-2.54),
        Property("Footprint", "JST-XH-4", 2, at_y=5.08),
    ], body=SymbolBody(
        rect=Rect(-5.08, -3.81, 5.08, 3.81),
        pins=build_mq7_pins(),
    )))

    # DS18B20
    symbols.append(Symbol("DS18B20", properties=[
        Property("Reference", "U", 0),
        Property("Value", "DS18B20", 1, at_y=-2.54),
        Property("Footprint", "TO-92", 2, at_y=5.08),
    ], body=SymbolBody(
        rect=Rect(-5.08, -3.81, 5.08, 3.81),
        pins=build_ds18b20_pins(),
    )))

    # BSS138
    symbols.append(Symbol("BSS138", properties=[
        Property("Reference", "Q", 0),
        Property("Value", "BSS138", 1, at_y=-2.54),
        Property("Footprint", "SOT-23", 2, at_y=5.08),
    ], body=SymbolBody(
        rect=Rect(-5.08, -3.81, 5.08, 3.81),
        pins=build_levelshifter_pins(),
    )))

    # USB-C
    symbols.append(Symbol("USB-C-31-SR", properties=[
        Property("Reference", "J", 0),
        Property("Value", "USB-C-31-SR", 1, at_y=-2.54),
        Property("Footprint", "USBC31SR", 2, at_y=5.08),
    ], body=SymbolBody(
        rect=Rect(-10.16, -7.62, 10.16, 7.62),
        pins=build_usb_pins(),
    )))

    # Terminal block 2-pin
    symbols.append(Symbol("TERM_BLOCK_2POS", properties=[
        Property("Reference", "J", 0),
        Property("Value", "TERM_BLOCK_2POS", 1, at_y=-2.54),
        Property("Footprint", "TerminalBlock_Phoenix_1x02", 2, at_y=5.08),
    ], body=SymbolBody(
        rect=Rect(-5.08, -3.81, 5.08, 3.81),
        pins=build_terminal_block_pins(2),
    )))

    # JST-XH 3-pin
    symbols.append(Symbol("JST-XH-3", properties=[
        Property("Reference", "J", 0),
        Property("Value", "JST-XH-3", 1, at_y=-2.54),
        Property("Footprint", "JST_XH_3", 2, at_y=5.08),
    ], body=SymbolBody(
        rect=Rect(-5.08, -5.08, 5.08, 5.08),
        pins=build_jst_pins(3),
    )))

    # JST-XH 4-pin
    symbols.append(Symbol("JST-XH-4", properties=[
        Property("Reference", "J", 0),
        Property("Value", "JST-XH-4", 1, at_y=-2.54),
        Property("Footprint", "JST_XH_4", 2, at_y=5.08),
    ], body=SymbolBody(
        rect=Rect(-5.08, -6.35, 5.08, 6.35),
        pins=build_jst_pins(4),
    )))

    # Expansion 2x10
    symbols.append(Symbol("EXPANSION_2X10", properties=[
        Property("Reference", "J", 0),
        Property("Value", "EXPANSION_2X10", 1, at_y=-2.54),
        Property("Footprint", "HDR_2X10", 2, at_y=5.08),
    ], body=SymbolBody(
        rect=Rect(-12.7, -20.32, 12.7, 20.32),
        pins=build_expansion_pins(),
    )))

    # Basic passives
    symbols.extend(build_basic_components())

    return symbols


def serialize_library(symbols):
    """Convert symbol list to KiCad symbol library text format."""
    lines = [
        "(kicad_symbol_lib",
        "  (version 20241018)",
        "  (generator kicad_sym)",
        "  (date 2026-05-29)",
        "  (lib (lib DieselFire)",
    ]
    for sym in symbols:
        lines.append("    " + sym.serialize())
    lines.extend(["  )", ")"])
    return "\n".join(lines)


def main():
    symbols = build_symbols()
    content = serialize_library(symbols)

    dest = BASE / "libraries" / "symbols" / "DieselFire.kicad_sym"
    dest.parent.mkdir(parents=True, exist_ok=True)
    with open(dest, "w") as f:
        f.write(content)
    print(f"Created: {dest} ({len(symbols)} symbols)")


if __name__ == "__main__":
    main()
