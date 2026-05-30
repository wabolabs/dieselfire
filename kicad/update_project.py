#!/usr/bin/env python3
"""Update KiCad project file to include custom library references."""

import json
from pathlib import Path

BASE = Path(__file__).parent
PROJ_FILE = BASE / "Afterburner-Modern.kicad_pro"

with open(PROJ_FILE, "r") as f:
    proj = json.load(f)

# Add symbol library references
proj["libraries"] = {
    "symbol_libraries": [
        {"uri": "project:Afterburner.kicad_sym"},
        {"uri": "/usr/share/kicad/symbols/Devices.kicad_sym"},
        {"uri": "/usr/share/kicad/symbols/Connectors.kicad_sym"},
        {"uri": "/usr/share/kicad/symbols/Connector_Adapters.kicad_sym"},
        {"uri": "/usr/share/kicad/symbols/Logic.kicad_sym"},
        {"uri": "/usr/share/kicad/symbols/Regulator_Linear.kicad_sym"},
        {"uri": "/usr/share/kicad/symbols/Transistor_BJT.kicad_sym"},
        {"uri": "/usr/share/kicad/symbols/Transistor_MOSFET.kicad_sym"},
        {"uri": "/usr/share/kicad/symbols/Diodes.kicad_sym"},
        {"uri": "/usr/share/kicad/symbols/Inductors.kicad_sym"},
        {"uri": "/usr/share/kicad/symbols/Resistors.kicad_sym"},
    ],
    "footprint_libraries": [
        {"uri": "project:Afterburner.kicad_fp"},
        {"uri": "/usr/share/kicad/symbols/Connector.pretty"},
        {"uri": "/usr/share/kicad/symbols/Connector_USB.pretty"},
        {"uri": "/usr/share/kicad/symbols/Connector_PinHeader_2.54mm.pretty"},
        {"uri": "/usr/share/kicad/symbols/Connector_PinSocket_2.54mm.pretty"},
        {"uri": "/usr/share/kicad/symbols/Connector_TerminalBlock.pretty"},
        {"uri": "/usr/share/kicad/symbols/SMD.pretty"},
        {"uri": "/usr/share/kicad/symbols/SMD_Packages.pretty"},
        {"uri": "/usr/share/kicad/symbols/Socket.pretty"},
    ],
}

with open(PROJ_FILE, "w") as f:
    json.dump(proj, f, indent=2)

print(f"Updated: {PROJ_FILE}")
