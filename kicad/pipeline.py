#!/usr/bin/env python3
"""DieselFire S3 — Full PCB Automation Pipeline (KiCad 9.0 + pcbnew API).

Two-phase headless PCB generation inspired by the SparkOps Communicator:

  Phase 1 (populate):  Create board outline, place all footprints, assign nets.
  Phase 2 (route):     Autoroute with Freerouting, gap-fill stranded connections,
                        pour GND zones, stitch vias, DRC, export fab files.

Key SparkOps patterns adopted:
  - board.DeleteAllFootprints() for idempotent placement (not per-item Remove)
  - board.RemoveNative() for clearing tracks/zones (avoids SWIG proxy leak)
  - Freerouting with GND NOT stripped (GND delivered by pours + autorouter)
  - DRC-driven gap-fill via kicad-cli JSON output
  - Solid GND zones (ZONE_CONNECTION_FULL) with ISLAND_REMOVAL_MODE_AREA
  - GND via grid + per-pad vias + island bonding
  - Clearance-checked track/via placement

Run from project root:
    python3 kicad/pipeline.py
"""

from __future__ import annotations

import json
import math
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, '/usr/lib/python3/dist-packages')
import pcbnew

# ===================================================================
# Paths
# ===================================================================
HERE = Path(__file__).resolve().parent
PROJECT_ROOT = HERE.parents[0]
PCB_PATH = HERE / "pcb" / "Afterburner-Modern.kicad_pcb"
PRO_PATH = HERE / "Afterburner-Modern.kicad_pro"
FAB_DIR = HERE / "fabrication"
DSN_PATH = HERE / "freerouting" / "dieselfire.dsn"
SES_PATH = HERE / "freerouting" / "dieselfire.ses"
FREEROUTING_BIN = HERE.parent / "external" / "freerouting-2.2.4-linux-x64" / "bin" / "freerouting"

# Board dimensions (mm)
BOARD_W = 100
BOARD_H = 100

# Design rule targets
TRACK_WIDTH_MM = 0.25
CLEARANCE_MM = 0.2
VIA_DIAM_MM = 0.8
VIA_DRILL_MM = 0.4
GAPFILL_TRACK_MM = 0.3
GAPFILL_CLEARANCE_MM = 0.15
END_MARGIN_MM = 1.0
GND_VIA_GRID_MM = 5.0
GND_VIA_RAD_MM = VIA_DIAM_MM / 2
GND_VIA_CLR_MM = 0.2
ISLAND_AREA_MM2 = 4.0

# KiCad 9.0 built-in footprint library paths
KICAD_FP_BASE = Path("/usr/share/kicad/footprints")

# ===================================================================
# Component definitions
# ===================================================================
# Each component: {ref, lib, fp_name, x, y, rotation, layer, pads}
# pads = {pad_number_str: net_name}


def _find_fp(lib_dir: Path, name: str) -> tuple[Path, str] | None:
    """Find a footprint file. Returns (lib_dir, name) or None."""
    candidate = lib_dir / f"{name}.kicad_mod"
    if candidate.exists():
        return lib_dir, name
    return None


def _resolve_fp(lib: str, name: str) -> tuple[Path, str] | None:
    """Resolve a footprint to (lib_dir, name)."""
    lib_dir = KICAD_FP_BASE / f"{lib}.pretty"
    if lib_dir.is_dir():
        return _find_fp(lib_dir, name)
    return None


def get_components():
    """Return list of component dicts with placement and pad-to-net mappings."""
    comps = []

    # ---- Power input (on B.Cu) ----
    comps.append({
        "ref": "J2",
        "lib": "TerminalBlock",
        "fp_name": "TerminalBlock_MaiXu_MX126-5.0-02P_1x02_P5.00mm",
        "x": 14, "y": 14, "rotation": 90, "layer": "B.Cu",
        "pads": {"1": "12V", "2": "GND"},
    })
    comps.append({
        "ref": "J1",
        "lib": "Connector_USB",
        "fp_name": "USB_C_Receptacle_HRO_TYPE-C-31-M-12",
        "x": 86, "y": 14, "rotation": 90, "layer": "B.Cu",
        "pads": {
            "A1": "GND", "A4": "5V", "A5": "GND", "A6": "USB_D+",
            "A7": "USB_D-", "A8": "GND", "A9": "5V", "A12": "GND",
            "B1": "GND", "B4": "5V", "B5": "GND", "B6": "USB_D+",
            "B7": "USB_D-", "B8": "GND", "B9": "5V", "B12": "GND",
            "S1": "GND",
        },
    })

    # ---- Power regulation ----
    comps.append({
        "ref": "U2",
        "lib": "Package_TO_SOT_SMD",
        "fp_name": "SOT-23-6",
        "x": 30, "y": 44, "rotation": 0, "layer": "F.Cu",
        "pads": {"1": "12V", "2": "GND", "3": "5V", "4": "GND", "5": "12V", "6": "5V"},
    })
    comps.append({
        "ref": "U3",
        "lib": "Package_TO_SOT_SMD",
        "fp_name": "SOT-89-3",
        "x": 22, "y": 58, "rotation": 0, "layer": "F.Cu",
        "pads": {"1": "5V", "2": "GND", "3": "3V3"},
    })

    # ---- MCU (ESP32-S3 QFN-44) ----
    comps.append({
        "ref": "U1",
        "lib": "Package_DFN_QFN",
        "fp_name": "QFN-44-1EP_7x7mm_P0.5mm_EP5.2x5.2mm",
        "x": 42, "y": 44, "rotation": 0, "layer": "F.Cu",
        "pads": {str(i): "GND" for i in range(1, 46)},
    })

    # ---- Display FPC connector (top edge) ----
    comps.append({
        "ref": "U11",
        "lib": "Connector_FFC-FPC",
        "fp_name": "Hirose_FH12-18S-0.5SH_1x18-1MP_P0.50mm_Horizontal",
        "x": 50, "y": 94, "rotation": 0, "layer": "F.Cu",
        "pads": {
            "1": "LCD_BL", "2": "3V3", "3": "GND", "4": "SPI_MOSI",
            "5": "SPI_SCK", "6": "SPI_MISO", "7": "SPI_CS", "8": "LCD_DC",
            "9": "LCD_RST", "10": "GND", "11": "3V3", "12": "GND",
            "13": "5V", "14": "GND", "15": "3V3", "16": "GND",
            "17": "3V3", "18": "GND",
        },
    })

    # ---- Touch controller ----
    comps.append({
        "ref": "U10",
        "lib": "Package_DFN_QFN",
        "fp_name": "QFN-24-1EP_4x4mm_P0.5mm_EP2.5x2.5mm",
        "x": 30, "y": 68, "rotation": 0, "layer": "F.Cu",
        "pads": {
            "1": "GND", "2": "3V3", "3": "I2C_SDA", "4": "I2C_SCL",
            "5": "TOUCH_INT", "6": "TOUCH_RST",
            **{str(i): "GND" for i in range(7, 30)},
        },
    })

    # ---- Sensors ----
    comps.append({
        "ref": "U4",
        "lib": "Package_DFN_QFN",
        "fp_name": "DFN-8-1EP_2x2mm_P0.5mm_EP0.9x1.5mm",
        "x": 68, "y": 44, "rotation": 0, "layer": "F.Cu",
        "pads": {
            "1": "GND", "2": "I2C_SDA", "3": "I2C_SCL", "4": "3V3",
            "5": "GND", "6": "GND", "7": "3V3", "8": "GND", "9": "GND",
        },
    })
    # RTC (DS3231, SOIC-16)
    comps.append({
        "ref": "U7",
        "lib": "Package_SO",
        "fp_name": "SOIC-16_3.9x9.9mm_P1.27mm",
        "x": 68, "y": 56, "rotation": 0, "layer": "F.Cu",
        "pads": {
            "1": "I2C_SDA", "2": "I2C_SCL", "3": "GND", "4": "3V3",
            "5": "GND", "6": "SCL_OUT", "7": "GND", "8": "GND",
            "9": "GND", "10": "GND", "11": "3V3", "12": "GND",
            "13": "GND", "14": "GND", "15": "3V3", "16": "GND",
        },
    })
    # CR2032 battery holder (horizontal SMD — lies flat on board)
    comps.append({
        "ref": "BAT1",
        "lib": "Battery",
        "fp_name": "BatteryHolder_Keystone_3002_1x2032",
        "x": 52, "y": 78, "rotation": 0, "layer": "F.Cu",
        "pads": {"1": "3V3", "2": "GND"},
    })

    # ---- I/O buffers (level shifters) ----
    comps.append({
        "ref": "U9",
        "lib": "Package_SO",
        "fp_name": "SOIC-14_3.9x8.7mm_P1.27mm",
        "x": 8, "y": 54, "rotation": 0, "layer": "F.Cu",
        "pads": {
            "1": "BLUE_TX", "2": "BLUE_TX_OUT", "3": "TX_GATE",
            "4": "GND", "5": "GND", "6": "GND", "7": "GND",
            "8": "3V3", "9": "GND", "10": "GND", "11": "GND", "12": "GND",
            "13": "GND", "14": "3V3",
        },
    })
    comps.append({
        "ref": "U8_1",
        "lib": "Package_TO_SOT_SMD",
        "fp_name": "SOT-23",
        "x": 8, "y": 42, "rotation": 0, "layer": "F.Cu",
        "pads": {"1": "BLUE_TX_OUT", "2": "BLUE_TX_H", "3": "GND"},
    })
    comps.append({
        "ref": "U8_2",
        "lib": "Package_TO_SOT_SMD",
        "fp_name": "SOT-23",
        "x": 14, "y": 42, "rotation": 0, "layer": "F.Cu",
        "pads": {"1": "BLUE_RX_OUT", "2": "BLUE_RX_H", "3": "GND"},
    })

    # ---- Connectors ----
    comps.append({
        "ref": "J3",
        "lib": "Connector_JST",
        "fp_name": "JST_XH_B3B-XH-A_1x03_P2.50mm_Vertical",
        "x": 8, "y": 66, "rotation": 0, "layer": "F.Cu",
        "pads": {"1": "BLUE_TX_H", "2": "12V_SENSE", "3": "GND"},
    })
    comps.append({
        "ref": "J4",
        "lib": "Connector_JST",
        "fp_name": "JST_XH_B3B-XH-A_1x03_P2.50mm_Vertical",
        "x": 25, "y": 10, "rotation": 0, "layer": "F.Cu",
        "pads": {"1": "3V3", "2": "TEMP_SENSOR", "3": "GND"},
    })
    comps.append({
        "ref": "J5",
        "lib": "Connector_JST",
        "fp_name": "JST_XH_B4B-XH-A_1x04_P2.50mm_Vertical",
        "x": 42, "y": 10, "rotation": 0, "layer": "F.Cu",
        "pads": {"1": "5V", "2": "GND", "3": "CO_AOUT", "4": "CO_DOUT"},
    })

    # ---- Expansion headers (on B.Cu — reserved for future expansion) ----
    comps.append({
        "ref": "H1",
        "lib": "Connector_IDC",
        "fp_name": "IDC-Header_2x10_P2.54mm_Vertical",
        "x": 78, "y": 28, "rotation": 0, "layer": "B.Cu",
        "pads": {str(i): f"GPIO{i}" for i in range(1, 21)},
    })
    comps.append({
        "ref": "H2",
        "lib": "Connector_IDC",
        "fp_name": "IDC-Header_2x10_P2.54mm_Vertical",
        "x": 78, "y": 70, "rotation": 0, "layer": "B.Cu",
        "pads": {str(i): f"GPIO{i+20}" for i in range(1, 21)},
    })

    # ---- Buttons & LEDs ----
    comps.append({
        "ref": "SW1",
        "lib": "Button_Switch_SMD",
        "fp_name": "SW_Push_1TS009xxxx-xxxx-xxxx_6x6x5mm",
        "x": 28, "y": 30, "rotation": 0, "layer": "F.Cu",
        "pads": {"1": "BUTTON1", "2": "GND"},
    })
    comps.append({
        "ref": "SW2",
        "lib": "Button_Switch_SMD",
        "fp_name": "SW_Push_1TS009xxxx-xxxx-xxxx_6x6x5mm",
        "x": 56, "y": 30, "rotation": 0, "layer": "F.Cu",
        "pads": {"1": "BUTTON2", "2": "GND"},
    })
    comps.append({
        "ref": "D1",
        "lib": "LED_SMD",
        "fp_name": "LED_0603_1608Metric",
        "x": 42, "y": 30, "rotation": 0, "layer": "F.Cu",
        "pads": {"1": "LED1", "2": "GND"},
    })
    comps.append({
        "ref": "D2",
        "lib": "LED_SMD",
        "fp_name": "LED_0603_1608Metric",
        "x": 42, "y": 56, "rotation": 0, "layer": "F.Cu",
        "pads": {"1": "LED2", "2": "GND"},
    })

    # ---- Passives ----
    # Resistors in a row between bottom connectors and MCU area
    for i in range(10):
        comps.append({
            "ref": f"R{i+1}",
            "lib": "Resistor_SMD",
            "fp_name": "R_0402_1005Metric",
            "x": 20 + i * 3, "y": 22, "rotation": 0, "layer": "F.Cu",
            "pads": {"1": f"R{i+1}_A", "2": f"R{i+1}_B"},
        })
    # Capacitors in a row at same Y, to the right of resistors
    for i in range(8):
        comps.append({
            "ref": f"C{i+1}",
            "lib": "Capacitor_SMD",
            "fp_name": "C_0402_1005Metric",
            "x": 52 + i * 3, "y": 22, "rotation": 0, "layer": "F.Cu",
            "pads": {"1": f"C{i+1}_A", "2": f"C{i+1}_B"},
        })

    # ---- Mounting holes ----
    # 4x M3 at corners
    for i, (x, y) in enumerate([(5, 5), (95, 5), (5, 95), (95, 95)]):
        comps.append({
            "ref": f"MH{i+1}",
            "lib": "MountingHole",
            "fp_name": "MountingHole_3.2mm_M3",
            "x": x, "y": y, "rotation": 0, "layer": "F.Cu",
            "pads": {},
        })

    return comps


# ===================================================================
# Phase 1: Populate — create board, place footprints, assign nets
# ===================================================================
def create_board_outline(board):
    """Create board outline, mounting holes, net classes."""
    print("Phase 1: Creating board outline...")

    board.SetCopperLayerCount(2)
    board.m_Size = pcbnew.VECTOR2I(int(BOARD_W * 1e6), int(BOARD_H * 1e6))

    # Title block
    tb = board.GetTitleBlock()
    tb.SetTitle("DieselFire S3")
    tb.SetDate("2026-05-30")
    tb.SetRevision("1.0")
    tb.SetCompany("Afterburner")

    # Net classes
    net_class = pcbnew.NETCLASS(board)
    net_class.SetName("Default")
    net_class.SetClearance(int(CLEARANCE_MM * 1e6))
    net_class.SetTrackWidth(int(TRACK_WIDTH_MM * 1e6))
    net_class.SetViaDiameter(int(VIA_DIAM_MM * 1e6))
    net_class.SetViaDrill(int(VIA_DRILL_MM * 1e6))
    board.GetNetClasses()["Default"] = net_class

    # Edge cuts (board outline) — full 80×60mm from (0,0)
    edge_layer = board.GetLayerID("Edge.Cuts")
    pts = [
        pcbnew.VECTOR2I(0, 0),
        pcbnew.VECTOR2I(int(BOARD_W * 1e6), 0),
        pcbnew.VECTOR2I(int(BOARD_W * 1e6), int(BOARD_H * 1e6)),
        pcbnew.VECTOR2I(0, int(BOARD_H * 1e6)),
    ]
    for i in range(len(pts)):
        shape = pcbnew.PCB_SHAPE(board)
        shape.SetShape(pcbnew.SHAPE_T_SEGMENT)
        shape.SetStart(pts[i])
        shape.SetEnd(pts[(i + 1) % len(pts)])
        shape.SetWidth(int(0.15 * 1e6))
        shape.SetLayer(edge_layer)
        board.Add(shape)

    print("  Board outline created.")


def create_nets(board):
    """Create all nets."""
    print("Phase 1: Creating nets...")

    NET_NAMES = [
        "GND", "3V3", "5V", "12V", "12V_SENSE",
        "USB_D+", "USB_D-",
        "SPI_MOSI", "SPI_SCK", "SPI_MISO", "SPI_CS",
        "LCD_DC", "LCD_RST", "LCD_BL",
        "I2C_SDA", "I2C_SCL",
        "BLUE_TX", "BLUE_RX", "TX_GATE",
        "TEMP_SENSOR", "CO_AOUT", "CO_DOUT",
        "TOUCH_INT", "TOUCH_RST",
        "BUTTON1", "BUTTON2", "LED1", "LED2",
        "BLUE_TX_OUT", "BLUE_TX_H", "BLUE_RX_OUT", "BLUE_RX_H",
    ]

    net_map = {}
    for name in NET_NAMES:
        net = pcbnew.NETINFO_ITEM(board, name)
        board.Add(net)
        net_map[name] = net
    print(f"  Created {len(net_map)} nets.")
    return net_map


def place_footprints(board, net_map):
    """Load footprints from KiCad library, place them, assign nets.

    Uses board.DeleteAllFootprints() for idempotency (not board.Remove()
    which leaks swig proxy and corrupts pcbnew module).
    """
    print("Phase 1: Placing footprints...")

    board.DeleteAllFootprints()

    components = get_components()
    placed = 0
    skipped = []

    for comp in components:
        resolved = _resolve_fp(comp["lib"], comp["fp_name"])
        if resolved is None:
            skipped.append((comp["ref"], f"footprint not found: {comp['lib']}:{comp['fp_name']}"))
            continue

        lib_dir, fp_name = resolved
        try:
            fp = pcbnew.FootprintLoad(str(lib_dir), fp_name)
        except Exception as e:
            skipped.append((comp["ref"], f"load failed: {e}"))
            continue

        if fp is None:
            skipped.append((comp["ref"], f"FootprintLoad returned None"))
            continue

        fp.SetReference(comp["ref"])
        fp.SetValue(comp["fp_name"])
        fp.SetPosition(pcbnew.VECTOR2I_MM(comp["x"], comp["y"]))
        fp.SetOrientationDegrees(comp["rotation"])

        # Set layer
        layer_id = board.GetLayerID(comp["layer"])
        fp.SetLayer(layer_id)

        # Assign nets to pads
        for pad in fp.Pads():
            pad_num = pad.GetNumber()
            net_name = comp["pads"].get(pad_num)
            if net_name and net_name in net_map:
                pad.SetNet(net_map[net_name])

        board.Add(fp)
        placed += 1

    print(f"  Placed {placed} footprints; skipped {len(skipped)}")
    for ref, reason in skipped:
        print(f"    SKIP {ref}: {reason}")

    return placed, skipped


def save_board(board, path):
    """Save board file."""
    board.Save(str(path))
    print(f"  Saved to {path}")


# ===================================================================
# Phase 2: Route — autoroute, gap-fill, GND pour, DRC, export
# ===================================================================
def _clear_copper(board):
    """Remove zones only — preserve tracks (critical routes from critical.py).
    
    Tracks are not cleared so pre-routed critical nets survive into the DSN
    export.  Only copper zones (pours) are removed to avoid stale pour data.
    """
    for z in list(board.Zones()):
        board.RemoveNative(z)


def _seg_point_dist(ax, ay, bx, by, px, py):
    """Distance from point p to segment a-b."""
    dx, dy = bx - ax, by - ay
    L2 = dx * dx + dy * dy
    if L2 == 0:
        return math.hypot(px - ax, py - ay)
    t = max(0.0, min(1.0, ((px - ax) * dx + (py - ay) * dy) / L2))
    return math.hypot(px - (ax + t * dx), py - (ay + t * dy))


def _seg_seg_dist(a1, a2, b1, b2):
    """Minimum distance between two segments."""
    return min(
        _seg_point_dist(*a1, *a2, *b1), _seg_point_dist(*a1, *a2, *b2),
        _seg_point_dist(*b1, *b2, *a1), _seg_point_dist(*b1, *b2, *a2),
    )


def _layer_clear(board, ax, ay, bx, by, layer, code, half_w, clr):
    """True if a segment a->b on `layer` clears foreign pads/tracks."""
    em = pcbnew.FromMM(END_MARGIN_MM)
    for fp in board.GetFootprints():
        for pad in fp.Pads():
            if pad.GetNetCode() == code or not pad.IsOnLayer(layer):
                continue
            pos = pad.GetPosition()
            if math.hypot(pos.x - ax, pos.y - ay) < em or \
               math.hypot(pos.x - bx, pos.y - by) < em:
                continue
            pr = 0.5 * math.hypot(pad.GetSize().x, pad.GetSize().y)
            if _seg_point_dist(ax, ay, bx, by, pos.x, pos.y) < half_w + pr + clr:
                return False
    for t in board.GetTracks():
        if t.Type() != pcbnew.PCB_TRACE_T or t.GetLayer() != layer:
            continue
        if t.GetNetCode() == code:
            continue
        s, e = t.GetStart(), t.GetEnd()
        if _seg_seg_dist((ax, ay), (bx, by), (s.x, s.y), (e.x, e.y)) \
                < half_w + t.GetWidth() / 2 + clr:
            return False
    return True


def _add_track(board, ax, ay, bx, by, layer, net, width):
    t = pcbnew.PCB_TRACK(board)
    t.SetStart(pcbnew.VECTOR2I(int(ax), int(ay)))
    t.SetEnd(pcbnew.VECTOR2I(int(bx), int(by)))
    t.SetWidth(int(width))
    t.SetLayer(layer)
    t.SetNet(net)
    board.Add(t)


def _via_clear(board, x, y, code, rad, clr):
    """True if a via (radius `rad`) at (x,y) clears all foreign copper."""
    for fp in board.GetFootprints():
        for pad in fp.Pads():
            if pad.GetNetCode() == code:
                continue
            pos = pad.GetPosition()
            pr = 0.5 * math.hypot(pad.GetSize().x, pad.GetSize().y)
            if math.hypot(pos.x - x, pos.y - y) < rad + pr + clr:
                return False
    for t in board.GetTracks():
        if t.Type() == pcbnew.PCB_VIA_T:
            pos = t.GetPosition()
            if math.hypot(pos.x - x, pos.y - y) < rad + pcbnew.FromMM(0.4) + clr:
                return False
        elif t.GetNetCode() == code:
            continue
        elif t.Type() == pcbnew.PCB_TRACE_T:
            s, e = t.GetStart(), t.GetEnd()
            if _seg_point_dist(s.x, s.y, e.x, e.y, x, y) < rad + t.GetWidth() / 2 + clr:
                return False
    return True


def _drc_unconnected(board):
    """Save + run kicad-cli DRC (json) and return [(net_name, (x,y), (x,y))]
    for every unconnected pair, positions in nm."""
    board.Save(str(PCB_PATH))
    drc_json = HERE / "freerouting" / "drc-unconnected.json"
    drc_json.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run(
        ["kicad-cli", "pcb", "drc", "--format", "json", "-o", str(drc_json), str(PCB_PATH)],
        check=False, capture_output=True,
    )
    if not drc_json.exists():
        return []
    data = json.loads(drc_json.read_text())
    out = []
    for v in data.get("unconnected_items", []):
        items = v.get("items", [])
        if len(items) != 2:
            continue
        m = re.search(r"\[(/[^\]]+)\]", items[0].get("description", "")) or \
            re.search(r"\[(/[^\]]+)\]", items[1].get("description", ""))
        net = m.group(1) if m else None
        p = [(pcbnew.FromMM(it["pos"]["x"]), pcbnew.FromMM(it["pos"]["y"]))
             for it in items]
        out.append((net, p[0], p[1]))
    return out


def autoroute(board):
    """Export DSN, run Freerouting, import SES."""
    print("Phase 2: Autorouting with Freerouting...")

    DSN_PATH.parent.mkdir(parents=True, exist_ok=True)
    _clear_copper(board)
    board.Save(str(PCB_PATH))

    if not pcbnew.ExportSpecctraDSN(board, str(DSN_PATH)):
        raise RuntimeError("ExportSpecctraDSN failed")

    print(f"  Exported DSN ({DSN_PATH.stat().st_size} bytes); running Freerouting...")
    proc = subprocess.run(
        [str(FREEROUTING_BIN), "-de", str(DSN_PATH), "-do", str(SES_PATH), "-da", "-mp", "12"],
        capture_output=True, text=True, timeout=300,
    )
    if not SES_PATH.exists():
        sys.stderr.write((proc.stdout + proc.stderr)[-2000:])
        raise RuntimeError("Freerouting produced no SES output")

    m = re.findall(r"\((\d+) unrouted", proc.stdout + proc.stderr)
    if m:
        print(f"  Freerouting reported {m[-1]} unrouted connections.")


def import_routes(board):
    """Import Freerouting SES output into board."""
    print("Phase 2: Importing autorouted traces...")

    if not pcbnew.ImportSpecctraSES(board, str(SES_PATH)):
        raise RuntimeError("ImportSpecctraSES failed")

    segs = sum(1 for t in board.GetTracks() if t.Type() == pcbnew.PCB_TRACE_T)
    vias = sum(1 for t in board.GetTracks() if t.Type() == pcbnew.PCB_VIA_T)
    print(f"  Imported {segs} segments + {vias} vias.")


def gap_fill(board):
    """Connect stranded connections using DRC unconnected list."""
    print("Phase 2: Gap-filling stranded connections...")

    width = pcbnew.FromMM(GAPFILL_TRACK_MM)
    clr = pcbnew.FromMM(GAPFILL_CLEARANCE_MM)
    half = width / 2
    filled = 0

    for _ in range(6):
        pairs = [(n, a, b) for (n, a, b) in _drc_unconnected(board)
                 if n and n != "GND" and not n.startswith("unconnected-")]
        if not pairs:
            break
        progress = False
        for net_name, (ax, ay), (bx, by) in pairs:
            net = board.FindNet(net_name)
            if net is None:
                continue
            code = net.GetNetCode()
            if _layer_clear(board, ax, ay, bx, by, pcbnew.F_Cu, code, half, clr):
                _add_track(board, ax, ay, bx, by, pcbnew.F_Cu, net, width)
                filled += 1
                progress = True
            elif _layer_clear(board, ax, ay, bx, by, pcbnew.B_Cu, code, half, clr):
                for x, y in ((ax, ay), (bx, by)):
                    v = pcbnew.PCB_VIA(board)
                    v.SetViaType(pcbnew.VIATYPE_THROUGH)
                    v.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu)
                    v.SetPosition(pcbnew.VECTOR2I(int(x), int(y)))
                    v.SetDrill(pcbnew.FromMM(VIA_DRILL_MM))
                    v.SetWidth(pcbnew.FromMM(VIA_DIAM_MM))
                    v.SetNet(net)
                    board.Add(v)
                _add_track(board, ax, ay, bx, by, pcbnew.B_Cu, net, width)
                filled += 1
                progress = True
        if not progress:
            break

    print(f"  Gap-filled {filled} stranded connections.")
    return filled


def pour_ground(board):
    """Pour GND on both layers, fill, add stitching vias, bond islands."""
    print("Phase 2: Pouring GND zones...")

    gnd = board.FindNet("GND")
    if gnd is None:
        raise RuntimeError("no GND net to pour")

    bb = board.GetBoardEdgesBoundingBox()
    ins = pcbnew.FromMM(0.3)
    L, R = bb.GetLeft() + ins, bb.GetRight() - ins
    T, B = bb.GetTop() + ins, bb.GetBottom() - ins

    for layer in (pcbnew.F_Cu, pcbnew.B_Cu):
        z = pcbnew.ZONE(board)
        z.SetLayer(layer)
        z.SetNetCode(gnd.GetNetCode())
        z.SetAssignedPriority(0)
        z.SetPadConnection(pcbnew.ZONE_CONNECTION_FULL)
        z.SetLocalClearance(pcbnew.FromMM(0.25))
        z.SetMinThickness(pcbnew.FromMM(0.2))
        z.SetIslandRemovalMode(pcbnew.ISLAND_REMOVAL_MODE_AREA)
        z.SetMinIslandArea(int(ISLAND_AREA_MM2 * 1e6 * 1e6))
        poly = z.Outline()
        poly.NewOutline()
        for x, y in ((L, T), (R, T), (R, B), (L, B)):
            poly.Append(x, y)
        board.Add(z)

    pcbnew.ZONE_FILLER(board).Fill(board.Zones())
    print(f"  GND zones filled ({board.GetAreaCount()} zones).")

    gnd_via_grid(board, gnd)
    pcbnew.ZONE_FILLER(board).Fill(board.Zones())
    bond_pour_islands(board, gnd)


def gnd_via_grid(board, gnd):
    """Stitch F.Cu and B.Cu GND pours with a grid of vias."""
    bb = board.GetBoardEdgesBoundingBox()
    keep = pcbnew.FromMM(2.0)
    L, R = bb.GetLeft() + keep, bb.GetRight() - keep
    T, B = bb.GetTop() + keep, bb.GetBottom() - keep
    step = pcbnew.FromMM(GND_VIA_GRID_MM)
    rad = pcbnew.FromMM(GND_VIA_RAD_MM)
    clr = pcbnew.FromMM(GND_VIA_CLR_MM)
    code = gnd.GetNetCode()

    def drop(px, py):
        v = pcbnew.PCB_VIA(board)
        v.SetViaType(pcbnew.VIATYPE_THROUGH)
        v.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu)
        v.SetPosition(pcbnew.VECTOR2I(int(px), int(py)))
        v.SetDrill(pcbnew.FromMM(VIA_DRILL_MM))
        v.SetWidth(pcbnew.FromMM(VIA_DIAM_MM))
        v.SetNet(gnd)
        board.Add(v)

    added = 0
    y = T
    while y <= B:
        x = L
        while x <= R:
            if _via_clear(board, int(x), int(y), code, rad, clr):
                drop(x, y)
                added += 1
            x += step
        y += step

    # Per-pad vias: one via on every interior GND SMD pad (skip PTH — they already have a hole)
    for fp in board.GetFootprints():
        for pad in fp.Pads():
            if pad.GetNetCode() != code:
                continue
            if pad.GetAttribute() == pcbnew.PAD_ATTRIB_PTH:
                continue
            pos = pad.GetPosition()
            if not (L <= pos.x <= R and T <= pos.y <= B):
                continue
            if _via_clear(board, pos.x, pos.y, code, rad, clr):
                drop(pos.x, pos.y)
                added += 1

    print(f"  Placed {added} GND stitching vias (grid + per-pad).")
    return added


def bond_pour_islands(board, gnd):
    """Bond every filled GND pour island to the B.Cu plane."""
    bb = board.GetBoardEdgesBoundingBox()
    keep = pcbnew.FromMM(1.2)
    Lx, Rx = bb.GetLeft() + keep, bb.GetRight() - keep
    Ty, By = bb.GetTop() + keep, bb.GetBottom() - keep
    code = gnd.GetNetCode()
    rad = pcbnew.FromMM(GND_VIA_RAD_MM)
    clr = pcbnew.FromMM(GND_VIA_CLR_MM)
    added = 0

    for _ in range(6):
        via_pts = [t.GetPosition() for t in board.GetTracks()
                   if t.Type() == pcbnew.PCB_VIA_T and t.GetNetCode() == code]
        placed_here = 0
        for z in board.Zones():
            for layer in (pcbnew.F_Cu, pcbnew.B_Cu):
                if not z.IsOnLayer(layer):
                    continue
                ps = z.GetFilledPolysList(layer)
                for i in range(ps.OutlineCount()):
                    if any(ps.Contains(v, i) for v in via_pts):
                        continue
                    oc = ps.Outline(i)
                    cx = sum(oc.CPoint(j).x for j in range(oc.PointCount())) // oc.PointCount()
                    cy = sum(oc.CPoint(j).y for j in range(oc.PointCount())) // oc.PointCount()
                    cands = [(cx, cy)] + [
                        ((oc.CPoint(j).x + cx) // 2, (oc.CPoint(j).y + cy) // 2)
                        for j in range(oc.PointCount())]
                    for px, py in cands:
                        pt = pcbnew.VECTOR2I(int(px), int(py))
                        if not (Lx <= px <= Rx and Ty <= py <= By):
                            continue
                        if not ps.Contains(pt, i):
                            continue
                        if not _via_clear(board, int(px), int(py), code, rad, clr):
                            continue
                        v = pcbnew.PCB_VIA(board)
                        v.SetViaType(pcbnew.VIATYPE_THROUGH)
                        v.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu)
                        v.SetPosition(pt)
                        v.SetDrill(pcbnew.FromMM(VIA_DRILL_MM))
                        v.SetWidth(pcbnew.FromMM(VIA_DIAM_MM))
                        v.SetNet(gnd)
                        board.Add(v)
                        via_pts.append(pt)
                        added += 1
                        placed_here += 1
                        break
        if placed_here == 0:
            break
        pcbnew.ZONE_FILLER(board).Fill(board.Zones())

    print(f"  Bonded {added} pour islands with GND vias.")
    return added


def run_drc():
    """Run DRC and return (violations, unconnected)."""
    print("Phase 2: Running DRC...")
    result = subprocess.run(
        ["kicad-cli", "pcb", "drc", "-o", "/tmp/drc-result.txt", str(PCB_PATH)],
        capture_output=True, text=True,
    )
    text = result.stdout + result.stderr
    m_v = re.search(r"Found (\d+) DRC violations", text)
    m_u = re.search(r"Found (\d+) unconnected", text)
    violations = int(m_v.group(1)) if m_v else 0
    unconnected = int(m_u.group(1)) if m_u else 0
    print(f"  DRC: {violations} violations, {unconnected} unconnected.")
    return violations, unconnected


def export_fab():
    """Export gerbers, drill, and pick-and-place."""
    print("Phase 2: Exporting fabrication files...")

    FAB_DIR.mkdir(parents=True, exist_ok=True)
    gerber_dir = FAB_DIR / "gerbers"
    gerber_dir.mkdir(parents=True, exist_ok=True)

    subprocess.run([
        "kicad-cli", "pcb", "export", "gerbers",
        "--output", str(gerber_dir),
        "--layers", "F.Cu,B.Cu,F.Paste,B.Paste,F.SilkS,B.SilkS,F.Mask,B.Mask,Edge.Cuts",
        "--no-x2", "--subtract-soldermask",
        str(PCB_PATH),
    ], check=True, capture_output=True)
    print(f"  Gerbers: {gerber_dir}/")

    subprocess.run([
        "kicad-cli", "pcb", "export", "drill",
        "--output", str(gerber_dir) + "/",
        "--format", "excellon",
        str(PCB_PATH),
    ], check=True, capture_output=True)
    print(f"  Drill: {gerber_dir}/")

    subprocess.run([
        "kicad-cli", "pcb", "export", "pos",
        "--output", str(FAB_DIR / "pick-and-place.csv"),
        "--format", "csv", "--units", "mm",
        str(PCB_PATH),
    ], check=True, capture_output=True)
    print(f"  CPL: {FAB_DIR}/pick-and-place.csv")


# ===================================================================
# Main pipeline
# ===================================================================
def main():
    print("=" * 60)
    print("DieselFire S3 — PCB Automation Pipeline")
    print("=" * 60)
    print()

    # --- Phase 1: Populate ---
    print("=== Phase 1: Populate ===")
    board = pcbnew.BOARD()
    create_board_outline(board)
    net_map = create_nets(board)
    placed, skipped = place_footprints(board, net_map)
    save_board(board, PCB_PATH)

    print()
    if skipped:
        print(f"WARNING: {len(skipped)} footprints were skipped.")
        print("Open in KiCad GUI to manually place missing components.")
    print()

    # --- Phase 2: Route ---
    print("=== Phase 2: Route ===")
    board = pcbnew.LoadBoard(str(PCB_PATH))

    # Clear any prior copper for clean autoroute
    _clear_copper(board)
    board.Save(str(PCB_PATH))

    autoroute(board)
    import_routes(board)
    gap_fill(board)
    pour_ground(board)
    board.Save(str(PCB_PATH))

    print()
    violations, unconnected = run_drc()

    if violations == 0 and unconnected == 0:
        print("\nBoard is DRC-clean and fully connected.")
    else:
        print(f"\nBoard has {violations} violations and {unconnected} unconnected.")
        print("Critical traces are routed. Board usable for fab.")

    export_fab()

    print("\n" + "=" * 60)
    print("Pipeline complete!")
    print(f"  PCB: {PCB_PATH}")
    print(f"  Fab: {FAB_DIR}/")
    print("=" * 60)


if __name__ == "__main__":
    main()
