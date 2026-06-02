#!/usr/bin/env python3
"""
Generate KiCad 9.0 PCB component modules for DieselFire - Part 2.
Builds all component footprints with pads and serializes to PCB file.
Appends to pcb/DieselFire.kicad_pcb.
"""

from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Tuple

BASE = Path(__file__).parent
PCB_DIR = BASE / "pcb"
PCB_FILE = PCB_DIR / "DieselFire.kicad_pcb"


@dataclass
class Pad:
    nr: int
    at_x: float
    at_y: float
    rotation: float = 0
    size_x: float = 1.0
    size_y: float = 1.0
    shape: str = "rect"
    layer: str = "F.Cu B.Cu"
    drill: float = 0.5
    net: int = 0
    remove_neighbor: bool = False
    clearance: float = 0.2

    def serialize(self):
        lines = []
        lines.append(f'    (pad {self.nr} {self.shape} pass_thru (at {self.at_x} {self.at_y} {self.rotation}) (size {self.size_x} {self.size_y})')
        lines.append(f'      (layers {self.layer}) (drill {self.drill}) (net {self.net})')
        if self.clearance > 0:
            lines.append(f'      (clearance {self.clearance})')
        lines.append(f'      (remove_unused_layers yes))')
        return "\n".join(lines)


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
    pads: List[Pad] = field(default_factory=list)
    uuid: str = ""

    def serialize(self):
        uuid_val = self.uuid if self.uuid else "00000000-0000-0000-0000-000000000000"
        locked_str = " locked" if self.locked else ""
        deleted_str = " deleted yes" if self.deleted else ""
        lines = []
        lines.append(f'  (module "{self.name}" (layer {self.layer}) (stub 1){locked_str}{deleted_str}')
        lines.append(f'    (attr {self.type})')
        if self.width > 0 and self.height > 0:
            lines.append(f'    (model ThirdParty')
            lines.append(f'      (type 3dobj)')
            lines.append(f'      (uri "") (offset (x 0 (y 0) (z 0))) (scale (x 1 (y 1) (z 1))) (rotate (x 0 (y 0) (z 0)))')
            lines.append(f'    )')
        if self.pads:
            for p in self.pads:
                lines.append(p.serialize())
        lines.append(f'  ) (uuid "{uuid_val}")')
        return "\n".join(lines)


def make_uuid(n):
    """Generate deterministic UUID from integer."""
    hex_str = format(n, '032x')
    return f"{hex_str[:8]}-{hex_str[8:12]}-{hex_str[12:16]}-{hex_str[16:20]}-{hex_str[20:32]}"


def build_esp32_module():
    """Build ESP32-S3 WROOM module footprint."""
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
    corners = [(-14, -18, 44), (-14, 18, 45), (14, -18, 46), (14, 18, 47)]
    for x, y, nr in corners:
        pads.append(Pad(nr=nr, at_x=x, at_y=y, size_x=2.5, size_y=2.5,
                        shape="circle", layer="F.Cu B.Cu", drill=1.6, net=0))

    return Module("ESP32_S3_WROOM", type="smd", at_x=-40, at_y=40,
                  pads=pads, width=28, height=36, uuid=make_uuid(4000))


def build_mp2451_module():
    """Build MP2451 buck converter module."""
    pads = [
        Pad(nr=1, at_x=-1.9, at_y=-1.3, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=3),
        Pad(nr=2, at_x=0, at_y=-1.9, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=0),
        Pad(nr=3, at_x=1.9, at_y=-1.3, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=4),
        Pad(nr=4, at_x=1.9, at_y=1.3, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=5),
        Pad(nr=5, at_x=0, at_y=1.9, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=6),
        Pad(nr=6, at_x=-1.9, at_y=1.3, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=7),
    ]
    return Module("MP2451_Module", type="smd", at_x=-120, at_y=80,
                  pads=pads, width=4.5, height=3.2, uuid=make_uuid(4001))


def build_ap2112_module():
    """Build AP2112 LDO module."""
    pads = [
        Pad(nr=1, at_x=-1.9, at_y=-1.3, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=8),
        Pad(nr=2, at_x=0, at_y=-1.9, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=0),
        Pad(nr=3, at_x=1.9, at_y=-1.3, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=9),
        Pad(nr=4, at_x=1.9, at_y=1.3, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=10),
        Pad(nr=5, at_x=0, at_y=1.9, size_x=1.0, size_y=0.6, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=1),
    ]
    return Module("AP2112_Module", type="smd", at_x=-80, at_y=80,
                  pads=pads, width=4.5, height=3.2, uuid=make_uuid(4002))


def build_bme280_module():
    """Build BME280 QFN-24 module."""
    pads = []
    half_pins = 12
    pitch = 0.5
    start_y = -(half_pins - 1) * pitch / 2
    for i in range(half_pins):
        y = start_y + i * pitch
        pads.append(Pad(nr=i+1, at_x=-2.0, at_y=y, size_x=0.3, size_y=1.2,
                        shape="rect", layer="F.Cu B.Cu", drill=0, net=i+1))
        pads.append(Pad(nr=i+half_pins+1, at_x=2.0, at_y=y, size_x=0.3, size_y=1.2,
                        shape="rect", layer="F.Cu B.Cu", drill=0, net=i+half_pins+1))
    return Module("BME280_Module", type="smd", at_x=0, at_y=-20,
                  pads=pads, width=4, height=4, uuid=make_uuid(4003))


def build_gt911_module():
    """Build GT911 touch controller module."""
    pads = []
    half_pins = 12
    pitch = 0.5
    start_y = -(half_pins - 1) * pitch / 2
    for i in range(half_pins):
        y = start_y + i * pitch
        pads.append(Pad(nr=i+1, at_x=-2.0, at_y=y, size_x=0.3, size_y=1.2,
                        shape="rect", layer="F.Cu B.Cu", drill=0, net=i+1))
        pads.append(Pad(nr=i+half_pins+1, at_x=2.0, at_y=y, size_x=0.3, size_y=1.2,
                        shape="rect", layer="F.Cu B.Cu", drill=0, net=i+half_pins+1))
    return Module("GT911_Module", type="smd", at_x=40, at_y=60,
                  pads=pads, width=4, height=4, uuid=make_uuid(4004))


def build_ds3231_module():
    """Build DS3231 RTC module."""
    pads = []
    half_pins = 8
    pitch = 1.27
    start_y = -(half_pins - 1) * pitch / 2
    for i in range(half_pins):
        y = start_y + i * pitch
        pads.append(Pad(nr=i+1, at_x=-2.0, at_y=y, size_x=0.6, size_y=2.0,
                        shape="rect", layer="F.Cu B.Cu", drill=0, net=i+1))
        pads.append(Pad(nr=i+half_pins+1, at_x=2.0, at_y=y, size_x=0.6, size_y=2.0,
                        shape="rect", layer="F.Cu B.Cu", drill=0, net=i+half_pins+1))
    return Module("DS3231_Module", type="smd", at_x=40, at_y=-20,
                  pads=pads, width=4, height=9.9, uuid=make_uuid(4005))


def build_mq7_module():
    """Build MQ-7 CO sensor module."""
    pads = [
        Pad(nr=1, at_x=-3.75, at_y=0, size_x=1.5, size_y=2.0, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=11),
        Pad(nr=2, at_x=-1.25, at_y=0, size_x=1.5, size_y=2.0, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=12),
        Pad(nr=3, at_x=1.25, at_y=0, size_x=1.5, size_y=2.0, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=13),
        Pad(nr=4, at_x=3.75, at_y=0, size_x=1.5, size_y=2.0, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=14),
    ]
    return Module("MQ7_Module", type="smd", at_x=-40, at_y=-60,
                  pads=pads, width=8, height=4, uuid=make_uuid(4006))


def build_ds18b20_module():
    """Build DS18B20 temperature sensor module."""
    pads = [
        Pad(nr=1, at_x=-1.5, at_y=-2.5, size_x=1.0, size_y=1.5, shape="circle",
            layer="F.Cu B.Cu", drill=0.8, net=15),
        Pad(nr=2, at_x=0, at_y=-3.0, size_x=1.0, size_y=1.5, shape="circle",
            layer="F.Cu B.Cu", drill=0.8, net=0),
        Pad(nr=3, at_x=1.5, at_y=-2.5, size_x=1.0, size_y=1.5, shape="circle",
            layer="F.Cu B.Cu", drill=0.8, net=16),
    ]
    return Module("DS18B20_Module", type="through_hole", at_x=0, at_y=-60,
                  pads=pads, width=8, height=6, uuid=make_uuid(4007))


def build_usbc_module():
    """Build USB-C connector module."""
    pads = [
        Pad(nr=1, at_x=-6.5, at_y=5.0, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=17),
        Pad(nr=2, at_x=-6.5, at_y=2.5, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=18),
        Pad(nr=3, at_x=-6.5, at_y=0, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=19),
        Pad(nr=4, at_x=-6.5, at_y=-2.5, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=20),
        Pad(nr=5, at_x=-6.5, at_y=-5.0, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=2),
        Pad(nr=6, at_x=-6.5, at_y=-7.5, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=0),
        Pad(nr=7, at_x=6.5, at_y=-7.5, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=0),
        Pad(nr=8, at_x=6.5, at_y=-5.0, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=2),
        Pad(nr=9, at_x=6.5, at_y=-2.5, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=21),
        Pad(nr=10, at_x=6.5, at_y=0, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=22),
        Pad(nr=11, at_x=6.5, at_y=2.5, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=23),
        Pad(nr=12, at_x=6.5, at_y=5.0, size_x=1.5, size_y=1.2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=24),
        Pad(nr=13, at_x=0, at_y=9.5, size_x=8, size_y=2, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=0),
    ]
    return Module("USBC31SR_Module", type="smd", at_x=-120, at_y=120,
                  pads=pads, width=15, height=20, uuid=make_uuid(4008))


def build_terminal_block_module():
    """Build 2-pin terminal block module."""
    pads = [
        Pad(nr=1, at_x=-2.5, at_y=0, size_x=3.0, size_y=3.5, shape="rect",
            layer="F.Cu B.Cu", drill=1.5, net=3),
        Pad(nr=2, at_x=2.5, at_y=0, size_x=3.0, size_y=3.5, shape="rect",
            layer="F.Cu B.Cu", drill=1.5, net=0),
    ]
    return Module("TerminalBlock_1x02_Module", type="through_hole", at_x=-120, at_y=40,
                  pads=pads, width=10, height=8, uuid=make_uuid(4009))


def build_jst_xh_3_module():
    """Build JST-XH 3-pin connector module."""
    pads = [
        Pad(nr=1, at_x=-2.5, at_y=0, size_x=1.5, size_y=2.0, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=25),
        Pad(nr=2, at_x=0, at_y=0, size_x=1.5, size_y=2.0, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=0),
        Pad(nr=3, at_x=2.5, at_y=0, size_x=1.5, size_y=2.0, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=26),
    ]
    return Module("JST_XH_3_Module", type="smd", at_x=-80, at_y=-60,
                  pads=pads, width=8, height=4, uuid=make_uuid(4010))


def build_jst_xh_4_module():
    """Build JST-XH 4-pin connector module."""
    pads = [
        Pad(nr=1, at_x=-3.75, at_y=0, size_x=1.5, size_y=2.0, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=27),
        Pad(nr=2, at_x=-1.25, at_y=0, size_x=1.5, size_y=2.0, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=28),
        Pad(nr=3, at_x=1.25, at_y=0, size_x=1.5, size_y=2.0, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=29),
        Pad(nr=4, at_x=3.75, at_y=0, size_x=1.5, size_y=2.0, shape="rect",
            layer="F.Cu B.Cu", drill=0.8, net=30),
    ]
    return Module("JST_XH_4_Module", type="smd", at_x=40, at_y=-60,
                  pads=pads, width=10, height=4, uuid=make_uuid(4011))


def build_expansion_2x10_module(offset_x=0):
    """Build 2x10 expansion header module."""
    pads = []
    for i in range(10):
        y = -12.7 + i * 2.54
        x = offset_x - 2.54
        pads.append(Pad(nr=i+1, at_x=x, at_y=y, size_x=1.6, size_y=1.6,
                        shape="circle", layer="F.Cu B.Cu", drill=1.0, net=i+1))
        x = offset_x + 2.54
        pads.append(Pad(nr=i+11, at_x=x, at_y=y, size_x=1.6, size_y=1.6,
                        shape="circle", layer="F.Cu B.Cu", drill=1.0, net=i+11))
    return Module("HDR_2X10_Module_" + str(offset_x), type="through_hole",
                  at_x=offset_x, at_y=-60, pads=pads, width=10, height=30,
                  uuid=make_uuid(4020 + offset_x // 10))


def build_passive_modules():
    """Build passive component modules."""
    modules = []

    # Resistors (0402)
    for i in range(8):
        x = -30 + i * 10
        pads = [
            Pad(nr=1, at_x=x-0.5, at_y=0, size_x=0.6, size_y=0.6, shape="rect",
                layer="F.Cu B.Cu", drill=0, net=100+i),
            Pad(nr=2, at_x=x+0.5, at_y=0, size_x=0.6, size_y=0.6, shape="rect",
                layer="F.Cu B.Cu", drill=0, net=110+i),
        ]
        modules.append(Module("R_0402_" + str(i), type="smd", at_x=x, at_y=20,
                              pads=pads, width=1.0, height=0.5, uuid=make_uuid(5000+i)))

    # Capacitors (0402)
    for i in range(12):
        x = -30 + i * 6
        pads = [
            Pad(nr=1, at_x=x-0.5, at_y=0, size_x=0.6, size_y=0.6, shape="rect",
                layer="F.Cu B.Cu", drill=0, net=200+i),
            Pad(nr=2, at_x=x+0.5, at_y=0, size_x=0.6, size_y=0.6, shape="rect",
                layer="F.Cu B.Cu", drill=0, net=210+i),
        ]
        modules.append(Module("C_0402_" + str(i), type="smd", at_x=x, at_y=-40,
                              pads=pads, width=1.0, height=0.5, uuid=make_uuid(5100+i)))

    # Inductor (0603)
    pads = [
        Pad(nr=1, at_x=-15-0.75, at_y=0, size_x=0.8, size_y=0.8, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=300),
        Pad(nr=2, at_x=-15+0.75, at_y=0, size_x=0.8, size_y=0.8, shape="rect",
            layer="F.Cu B.Cu", drill=0, net=301),
    ]
    modules.append(Module("L_0603_Module", type="smd", at_x=-15, at_y=100,
                          pads=pads, width=1.6, height=0.8, uuid=make_uuid(5200)))

    return modules


def build_modules():
    """Assemble all component modules."""
    modules = [
        build_esp32_module(),
        build_mp2451_module(),
        build_ap2112_module(),
        build_bme280_module(),
        build_gt911_module(),
        build_ds3231_module(),
        build_mq7_module(),
        build_ds18b20_module(),
        build_usbc_module(),
        build_terminal_block_module(),
        build_jst_xh_3_module(),
        build_jst_xh_4_module(),
        build_expansion_2x10_module(-100),
        build_expansion_2x10_module(100),
    ]
    modules.extend(build_passive_modules())
    return modules


def main():
    """Generate PCB component modules and append to PCB file."""
    modules = build_modules()

    # Read existing PCB file
    with open(PCB_FILE, "r") as f:
        existing = f.read()

    # Remove any existing modules section
    if '(modules' in existing:
        # Find the first (modules and remove everything from there to the end
        idx = existing.index('(modules')
        existing = existing[:idx].rstrip()

    # Append modules before the closing parenthesis
    module_lines = []
    for m in modules:
        module_lines.append(m.serialize())

    # Insert modules before the final ')'
    content = existing
    if content.endswith(')'):
        content = content[:-1]
    content += '\n\n  (modules\n'
    content += '\n\n'.join(module_lines)
    content += '\n  )\n)\n'

    with open(PCB_FILE, "w") as f:
        f.write(content)

    print(f"Updated: {PCB_FILE} ({len(modules)} modules added)")


if __name__ == "__main__":
    main()
