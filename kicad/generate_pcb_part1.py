#!/usr/bin/env python3
"""
Generate KiCad 9.0 PCB layout for DieselFire - Part 1.
Builds board header, zones, vias, and tracks from dataclasses.
Outputs to pcb/DieselFire.kicad_pcb (part 1).
"""

from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional, List, Tuple

BASE = Path(__file__).parent
PCB_DIR = BASE / "pcb"


@dataclass
class Point:
    x: float
    y: float


@dataclass
class Segment:
    start_x: float
    start_y: float
    end_x: float
    end_y: float
    width: float
    layer: str = "F.Cu"
    net: int = 0

    def serialize(self):
        return (f'      (segment (start {self.start_x} {self.start_y}) '
                f'(end {self.end_x} {self.end_y}) (width {self.width}) '
                f'(layer {self.layer}) (net {self.net}))')


@dataclass
class Arc:
    start_x: float
    start_y: float
    end_x: float
    end_y: float
    angle: float
    width: float
    layer: str = "F.SilkS"

    def serialize(self):
        return (f'      (arc (start {self.start_x} {self.start_y}) '
                f'(end {self.end_x} {self.end_y}) (angle {self.angle}) '
                f'(width {self.width}) (layer {self.layer}))')


@dataclass
class Via:
    at_x: float
    at_y: float
    drill: float
    diameter: float
    layer_pair: Tuple[str, str] = ("F.Cu", "B.Cu")
    net: int = 0

    def serialize(self):
        cut_str = ""
        return (f'    (via at {self.at_x} {self.at_y} (size {self.diameter}) '
                f'(drill {self.drill}) (layers {self.layer_pair[0]} {self.layer_pair[1]}) {cut_str}'
                f'(net {self.net}))')


@dataclass
class Zone:
    layer: str
    net: int
    name: str = "Default"
    keep_in_level: bool = False
    fill: bool = True
    thermal_gap: float = 0.3
    thermal_bridge_width: float = 0.3
    corner_shape: str = "chamfer"
    chamfer_ratio: float = 0.5

    def serialize_header(self):
        lines = []
        lines.append(f'  (zone (layer {self.layer}) (net {self.net}) (name {self.name}) (layer {self.layer})')
        lines.append(f'    (is_rule_locked no) (hide_polygons no) (connect_pads')
        if self.keep_in_level:
            lines.append(f'      (clearance 0.5))')
        else:
            lines.append(f'      (clearance 0))')
        lines.append(f'    (min_thickness {0.254}) (filled_areas_thickness no)')
        if self.fill:
            lines.append(f'    (fill yes (thermal_gap {self.thermal_gap}) (thermal_bridge_width {self.thermal_bridge_width}))')
        else:
            lines.append(f'    (fill no)')
        lines.append(f'    (corner_shape {self.corner_shape}) (chamfer_ratio {self.chamfer_ratio})')
        return "\n".join(lines)

    def serialize_regions(self, points: List[Tuple[float, float]]):
        """Serialize zone polygon points."""
        pts = " ".join([f"({x} {y})" for x, y in points])
        return f'    (filled_polygon (pts {pts}))'


@dataclass
class Text:
    content: str
    at_x: float
    at_y: float
    rotation: float = 0
    layer: str = "F.SilkS"
    size_x: float = 1.0
    size_y: float = 1.0
    thickness: float = 0.15
    hide: bool = False

    def serialize(self):
        hide_str = " hide" if self.hide else ""
        return (f'      (text "{self.content}" (at {self.at_x} {self.at_y} {self.rotation}) '
                f'(size {self.size_x} {self.size_y}) (thickness {self.thickness}) '
                f'(layer {self.layer}){hide_str})')


@dataclass
class Graphic:
    type: str  # line, circle, arc
    start_x: float
    start_y: float
    end_x: float = 0
    end_y: float = 0
    radius: float = 0
    angle: float = 0
    width: float = 0.15
    layer: str = "F.SilkS"

    def serialize(self):
        if self.type == "line":
            return (f'      (graphic (line (start {self.start_x} {self.start_y}) '
                    f'(end {self.end_x} {self.end_y}) (width {self.width}) '
                    f'(layer {self.layer})))')
        elif self.type == "circle":
            return (f'      (graphic (circle (center {self.start_x} {self.start_y}) '
                    f'(radius {self.radius}) (width {self.width}) '
                    f'(layer {self.layer})))')
        elif self.type == "arc":
            return (f'      (graphic (arc (start {self.start_x} {self.start_y}) '
                    f'(end {self.end_x} {self.end_y}) (angle {self.angle}) '
                    f'(width {self.width}) (layer {self.layer})))')
        return ""


@dataclass
class Module:
    name: str
    layer: str = "F.Cu"
    type: str = "smd"
    at_x: float = 0
    at_y: float = 0
    locked: bool = False
    deleted: bool = False
    width: float = 0
    height: float = 0
    pads: List = field(default_factory=list)
    segments: List[Segment] = field(default_factory=list)
    arcs: List[Arc] = field(default_factory=list)
    texts: List[Text] = field(default_factory=list)
    graphics: List[Graphic] = field(default_factory=list)
    net: int = -1
    net_name: str = ""
    uuid: str = ""

    def serialize(self):
        lines = []
        locked_str = " locked" if self.locked else ""
        deleted_str = " deleted yes" if self.deleted else ""
        lines.append(f'  (module "{self.name}" (layer {self.layer}) (stub 1){locked_str}{deleted_str}')
        lines.append(f'    (attr {self.type})')
        if self.width > 0 and self.height > 0:
            lines.append(f'    (model ThirdParty')
            lines.append(f'      (type 3dobj)')
            lines.append(f'      (uri "") (offset (x 0 (y 0) (z 0))) (scale (x 1 (y 1) (z 1))) (rotate (x 0 (y 0) (z 0)))')
            lines.append(f'    )')
        if self.texts:
            for t in self.texts:
                lines.append(t.serialize())
        if self.graphics:
            for g in self.graphics:
                lines.append(g.serialize())
        if self.segments:
            for s in self.segments:
                lines.append(s.serialize())
        if self.arcs:
            for a in self.arcs:
                lines.append(a.serialize())
        lines.append(f'  )')
        return "\n".join(lines)


def make_uuid(n):
    """Generate deterministic UUID from integer."""
    hex_str = format(n, '032x')
    return f"{hex_str[:8]}-{hex_str[8:12]}-{hex_str[12:16]}-{hex_str[16:20]}-{hex_str[20:32]}"


def build_zones():
    """Build copper zone definitions for the PCB."""
    # Board outline points (80mm x 60mm)
    outline = [
        (-40, -30), (40, -30), (40, 30), (-40, 30)
    ]

    zones = []
    # Bottom layer ground plane
    zones.append(Zone("B.Cu", 0, "GND_Bottom", fill=True, thermal_gap=0.3, thermal_bridge_width=0.3))
    zones[-1].regions = outline

    # Top layer ground plane
    zones.append(Zone("F.Cu", 0, "GND_Top", fill=True, thermal_gap=0.3, thermal_bridge_width=0.3))
    zones[-1].regions = outline

    # 3.3V zone on top
    zones.append(Zone("F.Cu", 1, "3V3_Top", fill=True, thermal_gap=0.25, thermal_bridge_width=0.3))
    zones[-1].regions = [
        (-35, -25), (-25, -25), (-25, -15), (-15, -15),
        (-15, 15), (-25, 15), (-25, 25), (-35, 25)
    ]

    # 5V zone on top
    zones.append(Zone("F.Cu", 2, "5V_Top", fill=True, thermal_gap=0.25, thermal_bridge_width=0.3))
    zones[-1].regions = [
        (-10, -25), (0, -25), (0, -15), (10, -15),
        (10, 15), (0, 15), (0, 25), (-10, 25)
    ]

    # 12V zone on top
    zones.append(Zone("F.Cu", 3, "12V_Top", fill=True, thermal_gap=0.3, thermal_bridge_width=0.4))
    zones[-1].regions = [
        (15, -25), (30, -25), (30, 25), (15, 25)
    ]

    return zones


def build_vias():
    """Build via definitions."""
    vias = []
    # Via grid for ground connections
    for x in range(-35, 36, 10):
        for y in range(-25, 26, 10):
            vias.append(Via(x, y, 0.6, 1.2, ("F.Cu", "B.Cu"), 0))
    return vias


def build_tracks():
    """Build track/segment definitions for routing."""
    tracks = []
    # 12V input traces (1mm wide)
    tracks.append(Segment(-30, -20, -20, -20, 1.0, "F.Cu", 3))
    tracks.append(Segment(-20, -20, -20, -10, 1.0, "F.Cu", 3))

    # 5V traces (0.5mm wide)
    tracks.append(Segment(-15, -10, -5, -10, 0.5, "F.Cu", 2))
    tracks.append(Segment(-5, -10, -5, 0, 0.5, "F.Cu", 2))

    # 3.3V traces (0.3mm wide)
    tracks.append(Segment(0, 0, 10, 0, 0.3, "F.Cu", 1))
    tracks.append(Segment(10, 0, 10, 10, 0.3, "F.Cu", 1))

    # UART traces to Blue Wire connector (0.3mm wide)
    tracks.append(Segment(-25, 15, -30, 15, 0.3, "F.Cu", 4))
    tracks.append(Segment(-30, 15, -30, 5, 0.3, "F.Cu", 4))

    # SPI traces (0.2mm wide)
    tracks.append(Segment(5, 5, 15, 5, 0.2, "F.Cu", 5))
    tracks.append(Segment(5, 10, 15, 10, 0.2, "F.Cu", 6))
    tracks.append(Segment(5, 15, 15, 15, 0.2, "F.Cu", 7))

    # I2C traces (0.2mm wide)
    tracks.append(Segment(5, -5, 15, -5, 0.2, "F.Cu", 8))
    tracks.append(Segment(5, -10, 15, -10, 0.2, "F.Cu", 9))

    # OneWire trace (0.2mm wide)
    tracks.append(Segment(20, -15, 30, -15, 0.2, "F.Cu", 10))

    # ADC trace for MQ-7 (0.3mm wide)
    tracks.append(Segment(20, -20, 25, -20, 0.3, "F.Cu", 11))

    return tracks


def build_silk_lines():
    """Build silkscreen outline and reference markers."""
    return [
        Graphic("line", -39.5, -29.5, 39.5, -29.5, 0.15, "F.SilkS"),
        Graphic("line", 39.5, -29.5, 39.5, 29.5, 0.15, "F.SilkS"),
        Graphic("line", 39.5, 29.5, -39.5, 29.5, 0.15, "F.SilkS"),
        Graphic("line", -39.5, 29.5, -39.5, -29.5, 0.15, "F.SilkS"),
    ]


def build_setup():
    """Build PCB setup section."""
    return {
        "current_layer": 1,
        "cache_directory": "",
        "board_color_mode": 1,
        "edge_layer": "Edge.Cuts",
        "copper_zones_width": 0.254,
        "board_thickness": 1.6,
        "copper_thickness": 0.035,
        "pcb_color": {
            "FrontCopper": "#b26600",
            "BackCopper": "#454552",
            "FrontSilk": "#ffffff",
            "BackSilk": "#ffffff",
            "FrontSolder": "#006600",
            "BackSolder": "#006600",
        },
        "layers": [
            {"name": "F.Cu", "type": "Top", "enabled": "1"},
            {"name": "B.Cu", "type": "Bottom", "enabled": "1"},
            {"name": "B.Adhes", "type": "User", "enabled": "0"},
            {"name": "F.Adhes", "type": "User", "enabled": "0"},
            {"name": "B.Paste", "type": "User", "enabled": "1"},
            {"name": "F.Paste", "type": "User", "enabled": "1"},
            {"name": "B.SilkS", "type": "User", "enabled": "1"},
            {"name": "F.SilkS", "type": "User", "enabled": "1"},
            {"name": "B.Mask", "type": "User", "enabled": "1"},
            {"name": "F.Mask", "type": "User", "enabled": "1"},
            {"name": "Dwgs.User", "type": "User", "enabled": "1"},
            {"name": "Cmts.User", "type": "User", "enabled": "1"},
            {"name": "Eco1.User", "type": "User", "enabled": "1"},
            {"name": "Eco2.User", "type": "User", "enabled": "1"},
            {"name": "Edge.Cuts", "type": "User", "enabled": "1"},
            {"name": "Margin", "type": "User", "enabled": "0"},
            {"name": "B.CrtYd", "type": "User", "enabled": "1"},
            {"name": "F.CrtYd", "type": "User", "enabled": "1"},
            {"name": "B.Fab", "type": "User", "enabled": "1"},
            {"name": "F.Fab", "type": "User", "enabled": "1"},
        ],
        "unlimited_dimensions": False,
        "mnemonic": "DieselFire",
        "edge_thickness": 0.15,
        "copper_line_width": [0.254, 0.254, 0.254, 0.254, 0.254, 0.254],
        "text_width": 0.0,
        "text_height": 0.0,
        "grid_options": {
            "auto": True,
            "user": 1.0,
            "last": 1.0,
            "units": "mm",
        },
        "segment_width": 0.254,
        "pcb_text_width": 0.3,
        "pcb_text_height": 0.8,
        "pcb_text_mode": 0,
        "edge_cuts_width": 0.15,
        "solder_mask_margin": 0,
        "solder_paste_margin": 0,
        "solder_paste_ratio": 0,
        "board_layout_mode": 0,
        "design_rules": [],
    }


def build_net_classes():
    """Build net class definitions."""
    return {
        "class": 0,
        "classes": [
            {
                "name": "Default",
                "clearance": "0.2",
                "track_width": "0.25",
                "via_diam": "0.8",
                "via_drill": "0.4",
                "pad_clearance": "0.2",
                "wire_width": "0.15",
            },
            {
                "name": "Power",
                "clearance": "0.3",
                "track_width": "1.0",
                "via_diam": "1.0",
                "via_drill": "0.5",
                "pad_clearance": "0.25",
                "wire_width": "0.3",
            },
            {
                "name": "3V3",
                "clearance": "0.2",
                "track_width": "0.3",
                "via_diam": "0.8",
                "via_drill": "0.4",
                "pad_clearance": "0.2",
                "wire_width": "0.15",
            },
            {
                "name": "5V",
                "clearance": "0.25",
                "track_width": "0.5",
                "via_diam": "0.9",
                "via_drill": "0.45",
                "pad_clearance": "0.2",
                "wire_width": "0.2",
            },
            {
                "name": "12V",
                "clearance": "0.3",
                "track_width": "1.0",
                "via_diam": "1.0",
                "via_drill": "0.5",
                "pad_clearance": "0.25",
                "wire_width": "0.3",
            },
            {
                "name": "SPI",
                "clearance": "0.2",
                "track_width": "0.2",
                "via_diam": "0.8",
                "via_drill": "0.4",
                "pad_clearance": "0.2",
                "wire_width": "0.15",
            },
            {
                "name": "I2C",
                "clearance": "0.2",
                "track_width": "0.2",
                "via_diam": "0.8",
                "via_drill": "0.4",
                "pad_clearance": "0.2",
                "wire_width": "0.15",
            },
            {
                "name": "UART",
                "clearance": "0.2",
                "track_width": "0.3",
                "via_diam": "0.8",
                "via_drill": "0.4",
                "pad_clearance": "0.2",
                "wire_width": "0.15",
            },
        ],
    }


def build_header():
    """Build the PCB file header."""
    lines = [
        '(kicad_pcb (version 20241018) (generator pcbnew)',
        '',
        '  (general',
        '    (thickness 1.6)',
        '    (legacy_teardrops no)',
        '    (pcb_edit_version 9.0)',
        '  )',
        '',
        '  (paper "A4")',
        '  (title_block',
        '    (title "DieselFire")',
        '    (date "2026-05-29")',
        '    (rev "1.0")',
        '    (company "DieselFire")',
        '    (comment 1 "ESP32-S3 Diesel Heater Controller")',
        '    (comment 2 "80mm x 60mm 2-Layer PCB")',
        '    (comment 3 "12V Input, 3.3V Logic")',
        '    (comment 4 "DieselFire Team")',
        '  )',
        '',
        '  (setup',
        '    (stackup (layer "F.Cu" (type "Top Copper") (thickness 0.035))',
        '      (layer "B.Cu" (type "Bottom Copper") (thickness 0.035))',
        '      (layer "B.SilkS" (type "Silk Screen"))',
        '      (layer "F.SilkS" (type "Silk Screen"))',
        '      (layer "B.Mask" (type "Solder Mask"))',
        '      (layer "F.Mask" (type "Solder Mask"))',
        '      (layer "B.Adhes" (type "Glue"))',
        '      (layer "F.Adhes" (type "Glue"))',
        '      (layer "B.Paste" (type "Solder Paste"))',
        '      (layer "F.Paste" (type "Solder Paste"))',
        '      (layer "B.CrtYd" (type "Courtyard"))',
        '      (layer "F.CrtYd" (type "Courtyard"))',
        '      (layer "B.Fab" (type "Fab"))',
        '      (layer "F.Fab" (type "Fab"))',
        '      (layer "B.Unused" (type "Unused"))',
        '      (layer "F.Unused" (type "Unused"))',
        '      (layer "Edge.Cuts" (type "Edge Cuts"))',
        '      (layer "Margin" (type "CantUse"))',
        '      (layer "Dwgs.User" (type "User"))',
        '      (layer "Cmts.User" (type "User"))',
        '      (layer "Eco1.User" (type "User"))',
        '      (layer "Eco2.User" (type "User"))',
        '      (material FR-4) (dielectric_count 1) (core_thickness 1.33))',
        '    (current_layer 1)',
        '    (cache_directory "")',
        '    (board_color_mode 1)',
        '    (edge_layer "Edge.Cuts")',
        '    (copper_zones_width 0.254)',
        '    (board_thickness 1.6)',
        '    (copper_thickness 0.035)',
        '    (unlimited_dimensions false)',
        '    (mnemonic "DieselFire")',
        '    (edge_thickness 0.15)',
        '    (copper_line_width [0.254 0.254 0.254 0.254 0.254 0.254])',
        '    (text_width 0.0)',
        '    (text_height 0.0)',
        '    (grid_options (auto true) (user 1.0) (last 1.0) (units "mm"))',
        '    (segment_width 0.254)',
        '    (pcb_text_width 0.3)',
        '    (pcb_text_height 0.8)',
        '    (pcb_text_mode 0)',
        '    (edge_cuts_width 0.15)',
        '    (solder_mask_margin 0)',
        '    (solder_paste_margin 0)',
        '    (solder_paste_ratio 0)',
        '    (board_layout_mode 0)',
        '  )',
        '',
    ]
    return lines


def build_net_classes_section(net_classes):
    """Build the net classes section."""
    cls = net_classes["class"]
    lines = ['  (net_classes', f'    (class {cls})']
    for nc in net_classes["classes"]:
        lines.append(f'      (name "{nc["name"]}")')
        lines.append(f'      (clearance "{nc["clearance"]}")')
        lines.append(f'      (track_width "{nc["track_width"]}")')
        lines.append(f'      (via_diam "{nc["via_diam"]}")')
        lines.append(f'      (via_drill "{nc["via_drill"]}")')
        lines.append(f'      (pad_clearance "{nc["pad_clearance"]}")')
        lines.append(f'      (wire_width "{nc["wire_width"]}")')
    lines.append('    )')
    lines.append('  )')
    lines.append('')
    return lines


def build_zones_section(zones):
    """Build the zones section."""
    uuid_val = make_uuid(1000)
    lines = ['  (zones (uuid "' + uuid_val + '")']
    for zone in zones:
        lines.append(zone.serialize_header())
        if hasattr(zone, 'regions') and zone.regions:
            lines.append(zone.serialize_regions(zone.regions))
        lines.append('  )')
    lines.append('')
    return lines


def build_vias_section(vias):
    """Build the vias section."""
    uuid1 = make_uuid(2000)
    uuid2 = make_uuid(2001)
    lines = ['  (via_def "thru_hole_via" (via_type thru) (via_type_def drill_diam 1.0) (uuid "' + uuid1 + '"))']
    lines.append(f'  (via_def "micro_hole_via" (via_type micro) (via_type_def drill_diam 0.25) (uuid "{uuid2}"))')
    lines.append('  (vias')
    for via in vias:
        lines.append(via.serialize())
    lines.append('  )')
    lines.append('')
    return lines


def build_tracks_section(tracks):
    """Build the tracks section."""
    uuid_val = make_uuid(3000)
    lines = ['  (tracks (uuid "' + uuid_val + '")']
    for track in tracks:
        lines.append(track.serialize())
    lines.append('  )')
    lines.append('')
    return lines


def main():
    """Generate PCB layout part 1."""
    zones = build_zones()
    vias = build_vias()
    tracks = build_tracks()
    net_classes = build_net_classes()

    lines = build_header()
    lines.extend(build_net_classes_section(net_classes))
    lines.extend(build_zones_section(zones))
    lines.extend(build_vias_section(vias))
    lines.extend(build_tracks_section(tracks))

    dest = PCB_DIR / "DieselFire.kicad_pcb"
    dest.parent.mkdir(parents=True, exist_ok=True)
    with open(dest, "w") as f:
        f.write("\n".join(lines))
    print(f"Created: {dest} (part 1 - header, zones, vias, tracks)")


if __name__ == "__main__":
    main()
