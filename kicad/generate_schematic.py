#!/usr/bin/env python3
"""
Generate KiCad 9.0 schematic file for DieselFire S3.
Uses dataclasses to define schematic elements, then serializes to KiCad text format.
"""

from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional, List, Tuple
from enum import Enum

BASE = Path(__file__).parent


class PinType(Enum):
    INPUT = "Input"
    OUTPUT = "Output"
    PASSIVE = "Passive"
    POWER_IN = "Power input"
    POWER_OUT = "Power output"


@dataclass
class SheetPosition:
    x: float
    y: float


@dataclass
class SheetSize:
    width: float
    height: float


@dataclass
class TextItem:
    content: str
    at_x: float
    at_y: float
    rotation: float = 0
    size_x: float = 1.27
    size_y: float = 1.27
    layer: str = "Title1"
    hide: bool = False

    def serialize(self):
        hide_str = " hide" if self.hide else ""
        return (f'(text "{self.content}" (at {self.at_x} {self.at_y} {self.rotation}) '
                f'(effects (font (size {self.size_x} {self.size_y})){hide_str}))')


@dataclass
class GraphicLine:
    start_x: float
    start_y: float
    end_x: float
    end_y: float
    width: float = 0.15
    color_r: int = 0
    color_g: int = 0
    color_b: int = 0
    color_a: int = 255
    layer: str = "User"

    def serialize(self):
        return (f'(graphic (line (start {self.start_x} {self.start_y}) '
                f'(end {self.end_x} {self.end_y}) (width {self.width})))')


@dataclass
class GraphicRect:
    start_x: float
    start_y: float
    end_x: float
    end_y: float
    width: float = 0.15
    fill: str = "None"
    layer: str = "User"

    def serialize(self):
        return (f'(graphic (rect (start {self.start_x} {self.start_y}) '
                f'(end {self.end_x} {self.end_y}) (width {self.width}) '
                f'(fill {self.fill}) (layer {self.layer})))')


@dataclass
class GraphicCircle:
    center_x: float
    center_y: float
    radius: float
    width: float = 0.15
    fill: str = "None"
    layer: str = "User"

    def serialize(self):
        return (f'(graphic (circle (center {self.center_x} {self.center_y}) '
                f'(radius {self.radius}) (width {self.width}) '
                f'(fill {self.fill}) (layer {self.layer})))')


@dataclass
class GraphicArc:
    center_x: float
    center_y: float
    start_x: float
    start_y: float
    end_x: float
    end_y: float
    angle: float
    width: float = 0.15
    layer: str = "User"

    def serialize(self):
        return (f'(graphic (arc (center {self.center_x} {self.center_y}) '
                f'(start {self.start_x} {self.start_y}) (end {self.end_x} {self.end_y}) '
                f'(angle {self.angle}) (width {self.width}) (layer {self.layer})))')


@dataclass
class GraphicPolygon:
    points: List[Tuple[float, float]]
    width: float = 0.15
    fill: str = "None"
    layer: str = "User"

    def serialize(self):
        pts = " ".join([f"({x} {y})" for x, y in self.points])
        return (f'(graphic (polygon (pts {pts}) (width {self.width}) '
                f'(fill {self.fill}) (layer {self.layer})))')


@dataclass
class SymbolInstance:
    lib_name: str
    part_name: str
    at_x: float
    at_y: float
    rotation: float = 0
    unit_count: int = 1
    deleted: bool = False
    uuid: str = ""

    def serialize(self):
        deleted_str = " deleted" if self.deleted else ""
        return (f'(symbol (lib_id "{self.lib_name}:{self.part_name}") '
                f'(at {self.at_x} {self.at_y} {self.rotation}) '
                f'(unit {self.unit_count}){deleted_str} (in_bom yes) (on_board yes) '
                f'(property "Reference" "?" (at {self.at_x} {self.at_y} {self.rotation}) '
                f'(effects (font (size 1.27 1.27)))) '
                f'(property "Value" "{self.part_name}" (at {self.at_x} {self.at_y - 10} {self.rotation}) '
                f'(effects (font (size 1.27 1.27)))) '
                f'(uuid "{self.uuid}"))')


@dataclass
class Wire:
    points: List[Tuple[float, float]]
    width: float = 0.15
    color_r: int = 0
    color_g: int = 0
    color_b: int = 0
    color_a: int = 255

    def serialize(self):
        pts = " ".join([f"({x} {y})" for x, y in self.points])
        return (f'(wire (pts {pts}) (width {self.width}) '
                f'(color {self.color_r} {self.color_g} {self.color_b} {self.color_a}))')


@dataclass
class Label:
    text: str
    at_x: float
    at_y: float
    rotation: float = 0
    size_x: float = 0.8
    size_y: float = 0.8
    layer: str = "Labels"
    effects_font_size: str = "0.8 0.8"

    def serialize(self):
        return (f'(label "{self.text}" (at {self.at_x} {self.at_y} {self.rotation}) '
                f'(effects (font (size {self.effects_font_size}))))')


@dataclass
class NoConnect:
    at_x: float
    at_y: float
    uuid: str = ""

    def serialize(self):
        return f'(no_connect (at {self.at_x} {self.at_y}) (uuid "{self.uuid}"))'


@dataclass
class BusEntry:
    start_x: float
    start_y: float
    end_x: float
    end_y: float
    width: float = 0.15
    layer: str = "Labels"

    def serialize(self):
        return (f'(bus_entry (start {self.start_x} {self.start_y}) '
                f'(end {self.end_x} {self.end_y}) (width {self.width}) '
                f'(layer {self.layer}))')


@dataclass
class HierarchicalLabel:
    text: str
    at_x: float
    at_y: float
    shape: str = "input"  # input, output, bidirectional, passive, power
    uuid: str = ""

    def serialize(self):
        return (f'(hierarchical_label "{self.text}" (shape {self.shape}) '
                f'(at {self.at_x} {self.at_y}) (uuid "{self.uuid}"))')


@dataclass
class HierarchicalSheet:
    path: str
    position_x: float
    position_y: float
    size_x: float
    size_y: float
    uuid: str = ""

    def serialize(self):
        return (f'(hierarchical_sheet (path /{self.path}) '
                f'(insertion hierarchical_label) (place_region (0 0 0 0)) '
                f'(insertion_pos ({self.position_x} {self.position_y})) '
                f'(size ({self.size_x} {self.size_y})) (uuid "{self.uuid}"))')


@dataclass
class HierarchicalSheetPath:
    path: str
    uuid: str = ""

    def serialize(self):
        return f'(hierarchical_sheet_path (path /{self.path}) (uuid "{self.uuid}"))'


@dataclass
class Schematic:
    title: str = "DieselFire S3"
    date: str = "2026-05-29"
    rev: str = "1.0"
    company: str = "Afterburner"
    comment_count: int = 4
    sheets: List[dict] = field(default_factory=list)
    symbols: List[SymbolInstance] = field(default_factory=list)
    wires: List[Wire] = field(default_factory=list)
    labels: List[Label] = field(default_factory=list)
    no_connects: List[NoConnect] = field(default_factory=list)
    bus_entries: List[BusEntry] = field(default_factory=list)
    hierarchical_labels: List[HierarchicalLabel] = field(default_factory=list)
    hierarchical_sheets: List[HierarchicalSheet] = field(default_factory=list)
    hierarchical_sheet_paths: List[HierarchicalSheetPath] = field(default_factory=list)
    graphic_items: List = field(default_factory=list)
    text_items: List[TextItem] = field(default_factory=list)
    juncs: List = field(default_factory=list)

    def serialize(self):
        lines = [
            '(kicad_sch (version 20241018) (generator kicad_sch)',
            '  (paper A4)',
            '  (title_block (title "DieselFire S3") (date "2026-05-29") (rev "1.0") (company "Afterburner"))',
        ]

        # Title block area
        lines.append('  (uuid "00000000-0000-0000-0000-000000000000")')

        # Symbols
        if self.symbols:
            for s in self.symbols:
                lines.append(f'  {s.serialize()}')

        # Wires
        if self.wires:
            for w in self.wires:
                lines.append(f'  {w.serialize()}')

        # Labels
        if self.labels:
            for l in self.labels:
                lines.append(f'  {l.serialize()}')

        # No connects
        if self.no_connects:
            for nc in self.no_connects:
                lines.append(f'  {nc.serialize()}')

        # Bus entries
        if self.bus_entries:
            for be in self.bus_entries:
                lines.append(f'  {be.serialize()}')

        # Hierarchical labels
        if self.hierarchical_labels:
            for hl in self.hierarchical_labels:
                lines.append(f'  {hl.serialize()}')

        # Hierarchical sheets
        if self.hierarchical_sheets:
            for hs in self.hierarchical_sheets:
                lines.append(f'  {hs.serialize()}')

        # Hierarchical sheet paths
        if self.hierarchical_sheet_paths:
            for hsp in self.hierarchical_sheet_paths:
                lines.append(f'  {hsp.serialize()}')

        # Junctions
        if self.juncs:
            for j in self.juncs:
                lines.append(f'  {j.serialize()}')

        lines.append(')')
        return "\n".join(lines)


def make_uuid(n):
    """Generate deterministic UUID."""
    hex_str = format(n, '032x')
    return f"{hex_str[:8]}-{hex_str[8:12]}-{hex_str[12:16]}-{hex_str[16:20]}-{hex_str[20:32]}"


def build_schematic():
    """Build the complete schematic with all components and connections."""
    schematic = Schematic()

    # Add title block text
    schematic.text_items = [
        TextItem("DieselFire S3", -100, 100, 0, 2.54, 2.54, "Title1"),
        TextItem("ESP32-S3 Based Diesel Heater Controller", -100, 80, 0, 1.27, 1.27, "Title1"),
        TextItem("Afterburner Team", -100, 60, 0, 1.27, 1.27, "Title1"),
        TextItem("2026-05-29", -100, 40, 0, 1.27, 1.27, "Title1"),
        TextItem("Sheet 1 of 1", -100, 20, 0, 1.27, 1.27, "Title1"),
    ]

    # Add border graphic
    schematic.graphic_items = [
        GraphicLine(-150, -150, 150, -150, 0.15, 0, 0, 0, 255, "User"),
        GraphicLine(150, -150, 150, 150, 0.15, 0, 0, 0, 255, "User"),
        GraphicLine(150, 150, -150, 150, 0.15, 0, 0, 0, 255, "User"),
        GraphicLine(-150, 150, -150, -150, 0.15, 0, 0, 0, 255, "User"),
    ]

    # Power section symbols
    power_x, power_y = -120, 80
    schematic.symbols.extend([
        SymbolInstance("Connector", "USB-C-31-SR", power_x - 40, power_y + 40, 0,
                       uuid=make_uuid(100)),
        SymbolInstance("Afterburner", "MP2451", power_x, power_y, 0,
                       uuid=make_uuid(101)),
        SymbolInstance("Afterburner", "AP2112", power_x + 40, power_y, 0,
                       uuid=make_uuid(102)),
        SymbolInstance("Afterburner", "TERM_BLOCK_2POS", power_x - 40, power_y - 40, 0,
                       uuid=make_uuid(103)),
    ])

    # MCU section
    mcu_x, mcu_y = -40, 40
    schematic.symbols.append(
        SymbolInstance("Afterburner", "ESP32-S3-WROOM-1-N8R8", mcu_x, mcu_y, 0,
                       uuid=make_uuid(200))
    )

    # Display section
    display_x, display_y = 40, 60
    schematic.symbols.append(
        SymbolInstance("Afterburner", "GT911", display_x, display_y, 0,
                       uuid=make_uuid(300))
    )

    # Sensor section
    sensor_x, sensor_y = 40, -20
    schematic.symbols.extend([
        SymbolInstance("Afterburner", "BME280", sensor_x, sensor_y, 0,
                       uuid=make_uuid(310)),
        SymbolInstance("Afterburner", "DS3231", sensor_x + 40, sensor_y, 0,
                       uuid=make_uuid(311)),
    ])

    # I/O section
    io_x, io_y = -40, -60
    schematic.symbols.extend([
        SymbolInstance("Afterburner", "MQ-7", io_x, io_y, 0,
                       uuid=make_uuid(400)),
        SymbolInstance("Afterburner", "DS18B20", io_x + 40, io_y, 0,
                       uuid=make_uuid(401)),
        SymbolInstance("Afterburner", "JST-XH-3", io_x - 40, io_y - 20, 0,
                       uuid=make_uuid(402)),
        SymbolInstance("Afterburner", "JST-XH-4", io_x + 40, io_y - 20, 0,
                       uuid=make_uuid(403)),
    ])

    # Expansion headers
    schematic.symbols.extend([
        SymbolInstance("Afterburner", "EXPANSION_2X10", -100, -60, 0,
                       uuid=make_uuid(500)),
        SymbolInstance("Afterburner", "EXPANSION_2X10", 100, -60, 0,
                       uuid=make_uuid(501)),
    ])

    # Level shifters for Blue Wire
    schematic.symbols.extend([
        SymbolInstance("Afterburner", "BSS138", -100, 20, 0,
                       uuid=make_uuid(600)),
        SymbolInstance("Afterburner", "BSS138", -100, 0, 0,
                       uuid=make_uuid(601)),
    ])

    # Power rails - labels
    schematic.labels.extend([
        Label("12V", power_x - 40, power_y - 20, 90, 0.8, 0.8, "Labels", "0.8 0.8"),
        Label("5V", power_x, power_y + 20, 90, 0.8, 0.8, "Labels", "0.8 0.8"),
        Label("3V3", power_x + 40, power_y + 20, 90, 0.8, 0.8, "Labels", "0.8 0.8"),
        Label("GND", power_x, power_y - 30, 0, 0.8, 0.8, "Labels", "0.8 0.8"),
    ])

    # UART/Blue Wire labels
    schematic.labels.extend([
        Label("BLUE_WIRE_TX", mcu_x - 20, mcu_y - 20, 0, 0.8, 0.8, "Labels", "0.8 0.8"),
        Label("BLUE_WIRE_RX", mcu_x - 20, mcu_y - 10, 0, 0.8, 0.8, "Labels", "0.8 0.8"),
        Label("UART0_TX", mcu_x - 10, mcu_y + 20, 0, 0.8, 0.8, "Labels", "0.8 0.8"),
        Label("UART0_RX", mcu_x - 10, mcu_y + 30, 0, 0.8, 0.8, "Labels", "0.8 0.8"),
    ])

    # SPI labels
    schematic.labels.extend([
        Label("SPI_MOSI", mcu_x + 20, mcu_y - 30, 0, 0.8, 0.8, "Labels", "0.8 0.8"),
        Label("SPI_MISO", mcu_x + 20, mcu_y - 40, 0, 0.8, 0.8, "Labels", "0.8 0.8"),
        Label("SPI_SCK", mcu_x + 20, mcu_y - 50, 0, 0.8, 0.8, "Labels", "0.8 0.8"),
        Label("SPI_CS", mcu_x + 20, mcu_y - 60, 0, 0.8, 0.8, "Labels", "0.8 0.8"),
        Label("LCD_RST", mcu_x + 20, mcu_y + 20, 0, 0.8, 0.8, "Labels", "0.8 0.8"),
        Label("LCD_DC", mcu_x + 20, mcu_y + 30, 0, 0.8, 0.8, "Labels", "0.8 0.8"),
    ])

    # I2C labels
    schematic.labels.extend([
        Label("I2C_SDA", mcu_x + 20, mcu_y + 50, 0, 0.8, 0.8, "Labels", "0.8 0.8"),
        Label("I2C_SCL", mcu_x + 20, mcu_y + 60, 0, 0.8, 0.8, "Labels", "0.8 0.8"),
    ])

    # I2C device labels
    schematic.labels.extend([
        Label("BME280", sensor_x, sensor_y - 10, 0, 0.8, 0.8, "Labels", "0.8 0.8"),
        Label("DS3231", sensor_x + 40, sensor_y - 10, 0, 0.8, 0.8, "Labels", "0.8 0.8"),
        Label("GT911", display_x, display_y - 10, 0, 0.8, 0.8, "Labels", "0.8 0.8"),
    ])

    # OneWire label
    schematic.labels.append(
        Label("TEMP_SENSOR", io_x + 40, io_y + 20, 0, 0.8, 0.8, "Labels", "0.8 0.8")
    )

    # ADC label
    schematic.labels.append(
        Label("CO_SENSOR", io_x, io_y + 20, 0, 0.8, 0.8, "Labels", "0.8 0.8")
    )

    # USB labels
    schematic.labels.extend([
        Label("USB_TX", power_x - 40, power_y + 20, 0, 0.8, 0.8, "Labels", "0.8 0.8"),
        Label("USB_RX", power_x - 40, power_y + 30, 0, 0.8, 0.8, "Labels", "0.8 0.8"),
    ])

    # No-connect markers for unused pins
    schematic.no_connects.extend([
        NoConnect(mcu_x + 30, mcu_y - 15, make_uuid(9000)),
        NoConnect(mcu_x + 30, mcu_y - 5, make_uuid(9001)),
        NoConnect(mcu_x + 30, mcu_y + 5, make_uuid(9002)),
        NoConnect(mcu_x + 30, mcu_y + 15, make_uuid(9003)),
    ])

    return schematic


def main():
    schematic = build_schematic()
    content = schematic.serialize()

    dest = BASE / "schematic" / "Afterburner-Modern.kicad_sch"
    dest.parent.mkdir(parents=True, exist_ok=True)
    with open(dest, "w") as f:
        f.write(content)
    print(f"Created: {dest}")


if __name__ == "__main__":
    main()
