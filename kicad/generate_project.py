#!/usr/bin/env python3
"""
Generate KiCad 9.0 project file for DieselFire.
Builds the JSON structure from Python dataclasses, then serializes.
"""

import json
from dataclasses import dataclass, asdict, field
from pathlib import Path
from typing import Optional

BASE = Path(__file__).parent


@dataclass
class Layer:
    name: str
    type: str
    enabled: str = "1"
    color: Optional[str] = None

    def to_dict(self):
        d = {"name": self.name, "type": self.type, "enabled": self.enabled}
        if self.color:
            d["color"] = self.color
        return d


@dataclass
class NetClass:
    name: str
    clearance: str
    track_width: str
    via_diam: str
    via_drill: str
    pad_clearance: str
    wire_width: str

    def to_dict(self):
        return asdict(self)


@dataclass
class Constraint:
    type: str
    order: int
    value: Optional[str] = None
    min: Optional[str] = None
    max: Optional[str] = None
    net_class: Optional[str] = None

    def to_dict(self):
        d = {"type": self.type, "order": self.order}
        if self.value:
            d["value"] = self.value
        if self.min:
            d["min"] = self.min
        if self.max:
            d["max"] = self.max
        if self.net_class:
            d["net_class"] = self.net_class
        return d


@dataclass
class DesignRules:
    name: str
    constraints: list

    def to_dict(self):
        return {"name": self.name, "constraints": [c.to_dict() for c in self.constraints]}


@dataclass
class Author:
    name: str
    email: str = ""

    def to_dict(self):
        d = {"name": self.name}
        if self.email:
            d["email"] = self.email
        return d


@dataclass
class Project:
    version: int = 1
    meta: dict = field(default_factory=lambda: {"format": "kicad_project"})
    design_settings: dict = field(default_factory=dict)
    authors: list = field(default_factory=list)
    net_settings: dict = field(default_factory=dict)
    pcbnew: dict = field(default_factory=dict)
    schematic: dict = field(default_factory=dict)

    def to_dict(self):
        return {
            "version": self.version,
            "meta": self.meta,
            "design_settings": self.design_settings,
            "authors": {"author": [a.to_dict() for a in self.authors]},
            "net_settings": self.net_settings,
            "pcbnew": self.pcbnew,
            "schematic": self.schematic,
        }


def build_layers():
    """Build the layer definitions for the project."""
    colored = [
        ("F.Cu", "Top", "#ff0000"),
        ("B.Cu", "Bottom", "#0000ff"),
        ("F.Paste", "User", "#ffffff"),
        ("B.Paste", "User", "#ffffff"),
        ("F.SilkS", "User", "#ffffff"),
        ("B.SilkS", "User", "#ffffff"),
        ("F.Mask", "User", "#00aa00"),
        ("B.Mask", "User", "#00aa00"),
        ("Dwgs.User", "User", "#cccccc"),
        ("Cmts.User", "User", "#cccccc"),
        ("Eco1.User", "User", "#cccccc"),
        ("Eco2.User", "User", "#cccccc"),
        ("Edge.Cuts", "User", "#ffffff"),
        ("F.CrtYd", "User", "#ff0000"),
        ("B.CrtYd", "User", "#ff0000"),
        ("F.Fab", "User", "#000000"),
        ("B.Fab", "User", "#000000"),
    ]
    disabled = [
        ("B.Adhes", "User"),
        ("F.Adhes", "User"),
        ("Margin", "User"),
    ]
    layers = []
    for name, typ, color in colored:
        layers.append(Layer(name, typ, "1", color))
    for name, typ in disabled:
        layers.append(Layer(name, typ, "0"))
    return [l.to_dict() for l in layers]


def build_net_settings():
    """Build net class definitions."""
    classes = [
        NetClass("Default", "0.2", "0.25", "0.8", "0.4", "0.2", "0.15"),
        NetClass("Power", "0.3", "1.0", "1.0", "0.5", "0.25", "0.3"),
        NetClass("3V3", "0.2", "0.3", "0.8", "0.4", "0.2", "0.15"),
        NetClass("5V", "0.25", "0.5", "0.9", "0.45", "0.2", "0.2"),
        NetClass("12V", "0.3", "1.0", "1.0", "0.5", "0.25", "0.3"),
        NetClass("SPI", "0.2", "0.2", "0.8", "0.4", "0.2", "0.15"),
        NetClass("I2C", "0.2", "0.2", "0.8", "0.4", "0.2", "0.15"),
        NetClass("UART", "0.2", "0.3", "0.8", "0.4", "0.2", "0.15"),
    ]
    return {
        "class": 0,
        "classes": [c.to_dict() for c in classes],
    }


def build_design_rules():
    """Build design rule constraints."""
    constraints = [
        Constraint("layer_range", 0),
        Constraint("pad_clearance", 100, "0.2"),
        Constraint("pad_to_pad", 110, "0.2"),
        Constraint("tracks_width", 200, "0.25"),
        Constraint("track_width", 300, "0.15", "1.0"),
        Constraint("via_annular_width", 310, "0.3"),
        Constraint("via_diameter", 320, "0.8"),
        Constraint("micro_via_diameter", 330, "0.25"),
        Constraint("pad_diameter", 340, min="0.4"),
        Constraint("copper_edge_clearance", 400, "0.5"),
        Constraint("net_class", 500, net_class="Default"),
    ]
    return {"default": DesignRules("Default", constraints).to_dict()}


def build_project():
    """Assemble the full project structure."""
    project = Project(
        version=1,
        meta={"format": "kicad_project"},
        design_settings={"defaults": {}},
        authors=[Author("DieselFire Team", "team@dieselfire.wabo.cc")],
        net_settings=build_net_settings(),
        pcbnew={
            "active_copper_layer_range": [0, 1],
            "layers": build_layers(),
        },
        schematic={
            "default_symbol_library": [
                "Device", "Connector", "Connector_Adapters", "Logic",
                "Regulator_Linear", "Transistor_BJT", "Transistor_MOSFET",
                "Diodes", "Inductors", "Resistors", "Power_Signal",
                "Connector_Generic", "Mechanical", "Connectors_Power",
                "Connectors_USB", "Connectors_TerminalBlock",
            ],
            "label_properties": {
                "italic": False, "bold": False, "font": "DejaVu",
                "size": 1.0, "height": 0.8,
            },
            "fields": {"fields_format": "$I $N $V $T"},
        },
    )
    project.design_settings["design_rules"] = build_design_rules()
    return project


def main():
    project = build_project()
    output = project.to_dict()

    dest = BASE / "DieselFire.kicad_pro"
    with open(dest, "w") as f:
        json.dump(output, f, indent=2)
    print(f"Created: {dest}")


if __name__ == "__main__":
    main()
