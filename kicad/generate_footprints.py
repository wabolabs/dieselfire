#!/usr/bin/env python3
"""
Generate KiCad 9.0 footprint library for DieselFire S3.
Uses dataclasses to define footprints, then serializes to KiCad text format.
"""

from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional, List, Tuple

BASE = Path(__file__).parent


@dataclass
class Point:
    x: float
    y: float


@dataclass
class Pad:
    nr: int
    at_x: float
    at_y: float
    rotation: float = 0
    size_x: float = 1.0
    size_y: float = 1.0
    shape: str = "rect"  # rect, circle, oval
    layer: str = "F.Cu B.Cu"
    drill: float = 0.5
    remove_neighbor: bool = False
    clearance: float = 0.2
    rect_rounding: float = 0.0
    net: str = ""

    def serialize(self):
        lines = []
        lines.append(f'    (pad {self.nr} {self.shape} pass_thru (at {self.at_x} {self.at_y} {self.rotation}) (size {self.size_x} {self.size_y})')
        if self.shape == "rect" and self.rect_rounding > 0:
            lines.append(f'      (rounding_ratio {self.rect_rounding})')
        lines.append(f'      (layers {self.layer}) (drill {self.drill})')
        if self.net:
            lines.append(f'      (net {self.net})')
        if self.clearance > 0:
            lines.append(f'      (clearance {self.clearance})')
        lines.append(f'      (remove_unused_layers yes))')
        return "\n".join(lines)


@dataclass
class Segment:
    start_x: float
    start_y: float
    end_x: float
    end_y: float
    width: float
    layer: str = "F.Cu"
    net: int = -1

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
    radius: float
    angle: float
    width: float
    layer: str = "F.SilkS"

    def serialize(self):
        return (f'      (arc (start {self.start_x} {self.start_y}) '
                f'(end {self.end_x} {self.end_y}) (angle {self.angle}) '
                f'(width {self.width}) (layer {self.layer}))')


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
        hide_str = "" if not self.hide else " hide"
        return (f'      (text "{self.content}" (at {self.at_x} {self.at_y} {self.rotation}) '
                f'(size {self.size_x} {self.size_y}) (thickness {self.thickness}) '
                f'(layer {self.layer}){hide_str})')


@dataclass
class Graphic:
    type: str  # line, circle
    start_x: float
    start_y: float
    end_x: float = 0
    end_y: float = 0
    width: float = 0.15
    layer: str = "F.SilkS"

    def serialize(self):
        if self.type == "line":
            return (f'      (graphic (line (start {self.start_x} {self.start_y}) '
                    f'(end {self.end_x} {self.end_y}) (width {self.width}) '
                    f'(layer {self.layer})))')
        elif self.type == "circle":
            return (f'      (graphic (circle (center {self.start_x} {self.start_y}) '
                    f'(radius {self.end_x}) (width {self.width}) '
                    f'(layer {self.layer})))')
        return ""


@dataclass
class Zone:
    layer: str
    net: int
    fill: bool = True
    keep_in_level: bool = False

    def serialize(self):
        lines = []
        lines.append(f'    (zone (layer {self.layer}) (net {self.net}) (name Default) (layer {self.layer})')
        lines.append(f'      (is_rule_locked no) (hide_polygons no) (connect_pads')
        if self.keep_in_level:
            lines.append(f'        (clearance 0.5))')
        else:
            lines.append(f'        (clearance 0))')
        lines.append(f'      (min_thickness {0.254}) (filled_areas_thickness no)')
        if self.fill:
            lines.append(f'      (fill yes (thermal_gap 0.3) (thermal_bridge_width 0.3))')
        else:
            lines.append(f'      (fill no)')
        return "\n".join(lines)


@dataclass
class Module:
    name: str
    layer: str = "F.Cu"
    type: str = "smd"  # through_hole, smd
    at_x: float = 0
    at_y: float = 0
    locked: bool = False
    deleted: bool = False
    locks: bool = True
    width: float = 0
    height: float = 0
    pads: List[Pad] = field(default_factory=list)
    segments: List[Segment] = field(default_factory=list)
    arcs: List[Arc] = field(default_factory=list)
    texts: List[Text] = field(default_factory=list)
    graphics: List[Graphic] = field(default_factory=list)
    zones: List[Zone] = field(default_factory=list)
    net: int = -1
    net_name: str = ""

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
        if self.pads:
            for p in self.pads:
                lines.append(p.serialize())
        if self.segments:
            for s in self.segments:
                lines.append(s.serialize())
        if self.arcs:
            for a in self.arcs:
                lines.append(a.serialize())
        lines.append(f'  )')
        return "\n".join(lines)


def make_smd_pad(nr, x, y, wx, wy, net_num=0):
    """Create an SMD pad."""
    return Pad(nr=nr, at_x=x, at_y=y, size_x=wx, size_y=wy, shape="rect",
               layer="F.Cu B.Cu", drill=0, net=net_num)

def make_th_pad(nr, x, y, drill, net_num=0, size_x=1.5, size_y=1.5):
    """Create a through-hole pad."""
    return Pad(nr=nr, at_x=x, at_y=y, size_x=size_x, size_y=size_y, shape="circle",
               layer="F.Cu B.Cu", drill=drill, net=net_num)

def make_circle_pad(nr, x, y, diameter, drill, net_num=0):
    """Create a circular pad."""
    return Pad(nr=nr, at_x=x, at_y=y, size_x=diameter, size_y=diameter, shape="circle",
               layer="F.Cu B.Cu", drill=drill, net=net_num)


def make_esp32_footprint():
    """Create ESP32-S3 WROOM footprint."""
    # Simplified module footprint - actual would use precise coordinates from datasheet
    pads = []
    # Left side - 14 pins
    for i in range(14):
        y = -15.24 + i * 2.54
        pads.append(Pad(nr=i+1, at_x=-11.0, at_y=y, size_x=1.5, size_y=0.8,
                        shape="rect", layer="F.Cu B.Cu", drill=0.8, net=i+1))
    # Right side - 14 pins
    for i in range(14):
        y = -15.24 + i * 2.54
        pads.append(Pad(nr=i+15, at_x=11.0, at_y=y, size_x=1.5, size_y=0.8,
                        shape="rect", layer="F.Cu B.Cu", drill=0.8, net=i+15))
    # Corner mounting pads
    corners = [(-14, -18), (-14, 18), (14, -18), (14, 18)]
    for i, (x, y) in enumerate(corners):
        pads.append(Pad(nr=44+i, at_x=x, at_y=y, size_x=2.5, size_y=2.5,
                        shape="circle", layer="F.Cu B.Cu", drill=1.6, net=1))

    texts = [
        Text("ESP32-S3-WROOM", 0, -22, 0, "F.SilkS", 1.0, 1.0),
        Text("U", 0, 22, 0, "F.Fab", 0.5, 0.5),
    ]

    graphics = [
        Graphic("line", -14, -18, 14, -18, 0.15, "F.SilkS"),
        Graphic("line", 14, -18, 14, 18, 0.15, "F.SilkS"),
        Graphic("line", 14, 18, -14, 18, 0.15, "F.SilkS"),
        Graphic("line", -14, 18, -14, -18, 0.15, "F.SilkS"),
    ]

    return Module("ESP32_S3_WROOM", pads=pads, texts=texts, graphics=graphics,
                  width=28, height=36)


def make_sot23_5_footprint():
    """Create SOT-23-5 footprint."""
    pads = [
        Pad(nr=1, at_x=-1.9, at_y=-1.3, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=1),
        Pad(nr=2, at_x=0, at_y=-1.9, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=2),
        Pad(nr=3, at_x=1.9, at_y=-1.3, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=3),
        Pad(nr=4, at_x=1.9, at_y=1.3, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=4),
        Pad(nr=5, at_x=0, at_y=1.9, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=5),
    ]
    texts = [Text("U", 0, -3.5, 0, "F.Fab", 0.5, 0.5)]
    return Module("SOT-23-5", pads=pads, texts=texts, width=4.5, height=3.2)


def make_sot23_6_footprint():
    """Create SOT-23-6 footprint."""
    pads = [
        Pad(nr=1, at_x=-1.9, at_y=-1.3, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=1),
        Pad(nr=2, at_x=0, at_y=-1.9, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=2),
        Pad(nr=3, at_x=1.9, at_y=-1.3, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=3),
        Pad(nr=4, at_x=1.9, at_y=1.3, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=4),
        Pad(nr=5, at_x=0, at_y=1.9, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=5),
        Pad(nr=6, at_x=-1.9, at_y=1.3, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=6),
    ]
    texts = [Text("U", 0, -3.5, 0, "F.Fab", 0.5, 0.5)]
    return Module("SOT-23-6", pads=pads, texts=texts, width=4.5, height=3.2)


def make_qfn_footprint(pin_count, pitch=0.5, body_w=4, body_h=4, pad_w=0.3, pad_h=1.2, center_pad=True):
    """Create a QFN footprint with configurable pins."""
    pads = []
    half_body = body_w / 2
    half_pins = pin_count // 2
    start_y = -(half_pins - 1) * pitch / 2

    # Left side
    for i in range(half_pins):
        y = start_y + i * pitch
        pads.append(Pad(nr=i+1, at_x=-half_body, at_y=y, size_x=pad_w, size_y=pad_h,
                        shape="rect", layer="F.Cu B.Cu", drill=0, net=i+1))
    # Right side
    for i in range(half_pins):
        y = start_y + i * pitch
        pads.append(Pad(nr=i+half_pins+1, at_x=half_body, at_y=y, size_x=pad_w, size_y=pad_h,
                        shape="rect", layer="F.Cu B.Cu", drill=0, net=i+half_pins+1))

    texts = [Text("U", 0, body_h/2 + 1, 0, "F.Fab", 0.5, 0.5)]

    graphics = [
        Graphic("line", -half_body, -body_h/2, half_body, -body_h/2, 0.1, "F.SilkS"),
        Graphic("line", half_body, -body_h/2, half_body, body_h/2, 0.1, "F.SilkS"),
        Graphic("line", half_body, body_h/2, -half_body, body_h/2, 0.1, "F.SilkS"),
        Graphic("line", -half_body, body_h/2, -half_body, -body_h/2, 0.1, "F.SilkS"),
    ]

    return Module(f"QFN-{pin_count}_{body_w}x{body_h}mm", pads=pads, texts=texts,
                  graphics=graphics, width=body_w + 2, height=body_h + 2)


def make_soic_footprint(pin_count, pitch=1.27, body_w=4, body_h=9.9, pad_w=0.6, pad_h=2.0):
    """Create a SOIC footprint."""
    pads = []
    half_body = body_w / 2
    half_pins = pin_count // 2
    start_y = -(half_pins - 1) * pitch / 2

    for i in range(half_pins):
        y = start_y + i * pitch
        pads.append(Pad(nr=i+1, at_x=-half_body, at_y=y, size_x=pad_w, size_y=pad_h,
                        shape="rect", layer="F.Cu B.Cu", drill=0, net=i+1))
        pads.append(Pad(nr=i+half_pins+1, at_x=half_body, at_y=y, size_x=pad_w, size_y=pad_h,
                        shape="rect", layer="F.Cu B.Cu", drill=0, net=i+half_pins+1))

    texts = [Text("U", 0, body_h/2 + 1, 0, "F.Fab", 0.5, 0.5)]
    return Module(f"SOIC-{pin_count}_{body_w}x{body_h}mm", pads=pads, texts=texts,
                  width=body_w + 2, height=body_h + 2)


def make_usbc_footprint():
    """Create USB-C connector footprint."""
    pads = [
        # Left side
        Pad(nr=1, at_x=-6.5, at_y=5.0, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=1),
        Pad(nr=2, at_x=-6.5, at_y=2.5, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=2),
        Pad(nr=3, at_x=-6.5, at_y=0, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=3),
        Pad(nr=4, at_x=-6.5, at_y=-2.5, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=4),
        Pad(nr=5, at_x=-6.5, at_y=-5.0, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=5),
        Pad(nr=6, at_x=-6.5, at_y=-7.5, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=6),
        # Right side
        Pad(nr=7, at_x=6.5, at_y=-7.5, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=7),
        Pad(nr=8, at_x=6.5, at_y=-5.0, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=8),
        Pad(nr=9, at_x=6.5, at_y=-2.5, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=9),
        Pad(nr=10, at_x=6.5, at_y=0, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=10),
        Pad(nr=11, at_x=6.5, at_y=2.5, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=11),
        Pad(nr=12, at_x=6.5, at_y=5.0, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=12),
        # Shield pads
        Pad(nr=13, at_x=0, at_y=9.5, size_x=8, size_y=2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=6),
    ]
    texts = [Text("J", 0, -11, 0, "F.Fab", 0.5, 0.5)]
    graphics = [
        Graphic("line", -7.5, -10, 7.5, -10, 0.1, "F.SilkS"),
        Graphic("line", 7.5, -10, 7.5, 10, 0.1, "F.SilkS"),
        Graphic("line", 7.5, 10, -7.5, 10, 0.1, "F.SilkS"),
        Graphic("line", -7.5, 10, -7.5, -10, 0.1, "F.SilkS"),
    ]
    return Module("USBC31SR", pads=pads, texts=texts, graphics=graphics,
                  width=15, height=20)


def make_terminal_block_2pin():
    """Create 2-pin terminal block footprint."""
    pads = [
        Pad(nr=1, at_x=-2.5, at_y=0, size_x=3.0, size_y=3.5, shape="rect",
            layer="F.Cu B.Cu", drill=1.5, net=1),
        Pad(nr=2, at_x=2.5, at_y=0, size_x=3.0, size_y=3.5, shape="rect",
            layer="F.Cu B.Cu", drill=1.5, net=2),
    ]
    texts = [Text("J", 0, -5, 0, "F.Fab", 0.5, 0.5)]
    graphics = [
        Graphic("line", -5, -4, 5, -4, 0.15, "F.SilkS"),
        Graphic("line", 5, -4, 5, 4, 0.15, "F.SilkS"),
        Graphic("line", 5, 4, -5, 4, 0.15, "F.SilkS"),
        Graphic("line", -5, 4, -5, -4, 0.15, "F.SilkS"),
    ]
    return Module("TerminalBlock_1x02", pads=pads, texts=texts, graphics=graphics,
                  width=10, height=8)


def make_jst_xh_3pin():
    """Create JST-XH 3-pin connector footprint."""
    pads = [
        Pad(nr=1, at_x=-2.5, at_y=0, size_x=1.5, size_y=2.0, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=1),
        Pad(nr=2, at_x=0, at_y=0, size_x=1.5, size_y=2.0, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=2),
        Pad(nr=3, at_x=2.5, at_y=0, size_x=1.5, size_y=2.0, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=3),
    ]
    texts = [Text("J", 0, -3.5, 0, "F.Fab", 0.5, 0.5)]
    graphics = [
        Graphic("line", -4, -2, 4, -2, 0.15, "F.SilkS"),
        Graphic("line", 4, -2, 4, 2, 0.15, "F.SilkS"),
        Graphic("line", 4, 2, -4, 2, 0.15, "F.SilkS"),
        Graphic("line", -4, 2, -4, -2, 0.15, "F.SilkS"),
    ]
    return Module("JST_XH_3", pads=pads, texts=texts, graphics=graphics,
                  width=8, height=4)


def make_jst_xh_4pin():
    """Create JST-XH 4-pin connector footprint."""
    pads = [
        Pad(nr=1, at_x=-3.75, at_y=0, size_x=1.5, size_y=2.0, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=1),
        Pad(nr=2, at_x=-1.25, at_y=0, size_x=1.5, size_y=2.0, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=2),
        Pad(nr=3, at_x=1.25, at_y=0, size_x=1.5, size_y=2.0, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=3),
        Pad(nr=4, at_x=3.75, at_y=0, size_x=1.5, size_y=2.0, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=4),
    ]
    texts = [Text("J", 0, -3.5, 0, "F.Fab", 0.5, 0.5)]
    graphics = [
        Graphic("line", -5, -2, 5, -2, 0.15, "F.SilkS"),
        Graphic("line", 5, -2, 5, 2, 0.15, "F.SilkS"),
        Graphic("line", 5, 2, -5, 2, 0.15, "F.SilkS"),
        Graphic("line", -5, 2, -5, -2, 0.15, "F.SilkS"),
    ]
    return Module("JST_XH_4", pads=pads, texts=texts, graphics=graphics,
                  width=10, height=4)


def make_expansion_2x10():
    """Create 2x10 expansion header footprint."""
    pads = []
    # Left column (pins 1-10)
    for i in range(10):
        y = -12.7 + i * 2.54
        pads.append(Pad(nr=i+1, at_x=-2.54, at_y=y, size_x=1.6, size_y=1.6,
                        shape="circle", layer="F.Cu B.Cu", drill=1.0, net=i+1))
    # Right column (pins 11-20)
    for i in range(10):
        y = -12.7 + i * 2.54
        pads.append(Pad(nr=i+11, at_x=2.54, at_y=y, size_x=1.6, size_y=1.6,
                        shape="circle", layer="F.Cu B.Cu", drill=1.0, net=i+11))

    texts = [Text("J", 0, -16, 0, "F.Fab", 0.5, 0.5)]
    graphics = [
        Graphic("line", -5, -15, 5, -15, 0.15, "F.SilkS"),
        Graphic("line", 5, -15, 5, 15, 0.15, "F.SilkS"),
        Graphic("line", 5, 15, -5, 15, 0.15, "F.SilkS"),
        Graphic("line", -5, 15, -5, -15, 0.15, "F.SilkS"),
    ]
    return Module("HDR_2X10", pads=pads, texts=texts, graphics=graphics,
                  width=10, height=30)


def make_resistor_0402():
    """Create 0402 resistor footprint."""
    pads = [
        Pad(nr=1, at_x=-0.5, at_y=0, size_x=0.6, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=1),
        Pad(nr=2, at_x=0.5, at_y=0, size_x=0.6, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=2),
    ]
    return Module("R_0402", pads=pads, width=1.0, height=0.5)


def make_capacitor_0402():
    """Create 0402 capacitor footprint."""
    pads = [
        Pad(nr=1, at_x=-0.5, at_y=0, size_x=0.6, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=1),
        Pad(nr=2, at_x=0.5, at_y=0, size_x=0.6, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=2),
    ]
    return Module("C_0402", pads=pads, width=1.0, height=0.5)


def make_inductor_0603():
    """Create 0603 inductor footprint."""
    pads = [
        Pad(nr=1, at_x=-0.75, at_y=0, size_x=0.8, size_y=0.8, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=1),
        Pad(nr=2, at_x=0.75, at_y=0, size_x=0.8, size_y=0.8, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=2),
    ]
    return Module("L_0603", pads=pads, width=1.6, height=0.8)


def make_diode_sod923():
    """Create SOD-923 diode footprint (BAT54S)."""
    pads = [
        Pad(nr=1, at_x=-0.4, at_y=0, size_x=0.5, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=1),
        Pad(nr=2, at_x=0.4, at_y=0, size_x=0.5, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=2),
    ]
    return Module("SOD-923", pads=pads, width=1.0, height=0.5)


def make_crystal_32khz():
    """Create 32kHz crystal footprint (HC-49S)."""
    pads = [
        Pad(nr=1, at_x=-3.0, at_y=0, size_x=1.5, size_y=2.0, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=1),
        Pad(nr=2, at_x=3.0, at_y=0, size_x=1.5, size_y=2.0, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=2),
    ]
    texts = [Text("Y", 0, -5, 0, "F.Fab", 0.5, 0.5)]
    return Module("HC-49S", pads=pads, texts=texts, width=7, height=3)


def make_to92():
    """Create TO-92 package footprint (DS18B20)."""
    pads = [
        Pad(nr=1, at_x=-1.5, at_y=-2.5, size_x=1.0, size_y=1.5, shape="circle",
            layer="F.Cu B.Cu", drill=0.8, net=1),
        Pad(nr=2, at_x=0, at_y=-3.0, size_x=1.0, size_y=1.5, shape="circle",
            layer="F.Cu B.Cu", drill=0.8, net=2),
        Pad(nr=3, at_x=1.5, at_y=-2.5, size_x=1.0, size_y=1.5, shape="circle",
            layer="F.Cu B.Cu", drill=0.8, net=3),
    ]
    texts = [Text("U", 0, 3, 0, "F.Fab", 0.5, 0.5)]
    graphics = [
        Graphic("line", -4, -4, 4, -4, 0.15, "F.SilkS"),
        Graphic("circle", 0, -2, 4, 0, 0.15, "F.SilkS"),
    ]
    return Module("TO-92", pads=pads, texts=texts, graphics=graphics,
                  width=8, height=6)


def make_bss138_footprint():
    """Create SOT-23 for BSS138."""
    return make_sot23_5_footprint()


def make_footprint_library():
    """Assemble all footprints."""
    footprints = [
        make_esp32_footprint(),
        make_sot23_5_footprint(),
        make_sot23_6_footprint(),
        make_qfn_footprint(24, pitch=0.5, body_w=4, body_h=4, pad_w=0.3, pad_h=1.2),
        make_soic_footprint(16, pitch=1.27, body_w=4, body_h=9.9),
        make_usbc_footprint(),
        make_terminal_block_2pin(),
        make_jst_xh_3pin(),
        make_jst_xh_4pin(),
        make_expansion_2x10(),
        make_resistor_0402(),
        make_capacitor_0402(),
        make_inductor_0603(),
        make_diode_sod923(),
        make_crystal_32khz(),
        make_to92(),
        make_bss138_footprint(),
    ]
    return footprints


def serialize_footprint_library(footprints):
    """Convert footprint list to KiCad footprint library text format."""
    lines = [
        "(kicad_footprint_lib",
        "  (version 20241018)",
        "  (generator kicad_footprint)",
        "  (date 2026-05-29)",
        "  (lib (lib Afterburner)",
    ]
    for fp in footprints:
        lines.append(fp.serialize())
    lines.extend(["  )", ")"])
    return "\n".join(lines)


def main():
    footprints = make_footprint_library()
    content = serialize_footprint_library(footprints)

    dest = BASE / "footprints" / "Afterburner.kicad_fp"
    dest.parent.mkdir(parents=True, exist_ok=True)
    with open(dest, "w") as f:
        f.write(content)
    print(f"Created: {dest} ({len(footprints)} footprints)")


if __name__ == "__main__":
    main()
