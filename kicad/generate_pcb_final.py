#!/usr/bin/env python3
"""
Generate complete KiCad 9.0 PCB layout for DieselFire.
Places all components with proper footprints, adds zones, vias, silkscreen, and mounting holes.
"""

import sys
sys.path.insert(0, '/usr/lib/python3/dist-packages')
import pcbnew
from pathlib import Path

BASE = Path(__file__).parent
PCB_FILE = BASE / "pcb" / "DieselFire.kicad_pcb"


def create_board():
    """Create board with proper setup."""
    board = pcbnew.BOARD()
    board.SetCopperLayerCount(2)
    board.m_Size = pcbnew.VECTOR2I(80000, 60000)  # 80mm x 60mm

    tb = board.GetTitleBlock()
    tb.SetTitle("DieselFire")
    tb.SetDate("2026-05-29")
    tb.SetRevision("1.0")
    tb.SetCompany("DieselFire")

    # Net classes
    net_class = pcbnew.NETCLASS(board)
    net_class.SetName("Default")
    net_class.SetClearance(int(0.2 * 1e6))
    net_class.SetTrackWidth(int(0.25 * 1e6))
    net_class.SetViaDiameter(int(0.8 * 1e6))
    net_class.SetViaDrill(int(0.4 * 1e6))
    net_class.SetuViaDiameter(int(0.3 * 1e6))
    net_class.SetuViaDrill(int(0.1 * 1e6))
    board.GetNetClasses()["Default"] = net_class

    return board


def make_lset(board):
    """Create layer set for F.Cu and B.Cu."""
    lset = pcbnew.LSET()
    lset.AddLayer(pcbnew.F_Cu)
    lset.AddLayer(pcbnew.B_Cu)
    return lset


def add_smd_pad(fp, nr, x, y, wx, wy, net_code):
    """Add an SMD pad to a footprint."""
    pad = pcbnew.PAD(fp)
    pad.SetPosition(pcbnew.VECTOR2I(x, y))
    pad.SetSize(pcbnew.VECTOR2I(int(wx * 1e6), int(wy * 1e6)))
    pad.SetShape(pcbnew.PAD_SHAPE_RECT)
    pad.SetAttribute(pcbnew.PAD_ATTRIB_SMD)
    pad.SetLayerSet(make_lset(fp.GetBoard()))
    pad.SetDrillSize(pcbnew.VECTOR2I(0, 0))
    pad.SetNetCode(net_code)
    fp.Add(pad)


def add_th_pad(fp, nr, x, y, drill, net_code):
    """Add a through-hole pad to a footprint."""
    pad = pcbnew.PAD(fp)
    pad.SetPosition(pcbnew.VECTOR2I(x, y))
    pad.SetSize(pcbnew.VECTOR2I(int(1.5 * 1e6), int(1.5 * 1e6)))
    pad.SetShape(pcbnew.PAD_SHAPE_CIRCLE)
    pad.SetAttribute(pcbnew.PAD_ATTRIB_NPTH)
    pad.SetLayerSet(make_lset(fp.GetBoard()))
    pad.SetDrillSize(pcbnew.VECTOR2I(int(drill * 1e6), int(drill * 1e6)))
    pad.SetNetCode(net_code)
    fp.Add(pad)


def create_footprint(board, lib_id, ref, value, x, y, pads):
    """Create a footprint with given pads."""
    fp = pcbnew.FOOTPRINT(board)
    fp.SetPosition(pcbnew.VECTOR2I(int(x * 1e6), int(y * 1e6)))
    fp.SetFPID(pcbnew.LIB_ID(lib_id, lib_id))
    fp.SetAttributes(pcbnew.PAD_ATTRIB_SMD)
    fp.SetReference(ref)
    fp.SetValue(value)

    for nr, px, py, wx, wy, net_code in pads:
        add_smd_pad(fp, nr, int(px * 1e6), int(py * 1e6), wx, wy, net_code)

    board.Add(fp)
    return fp


def create_th_footprint(board, lib_id, ref, value, x, y, pads):
    """Create a through-hole footprint with given pads."""
    fp = pcbnew.FOOTPRINT(board)
    fp.SetPosition(pcbnew.VECTOR2I(int(x * 1e6), int(y * 1e6)))
    fp.SetFPID(pcbnew.LIB_ID(lib_id, lib_id))
    fp.SetAttributes(pcbnew.PAD_ATTRIB_NPTH)
    fp.SetReference(ref)
    fp.SetValue(value)

    for nr, px, py, drill, net_code in pads:
        add_th_pad(fp, nr, int(px * 1e6), int(py * 1e6), drill, net_code)

    board.Add(fp)
    return fp


def add_edge_cuts(board):
    """Add board outline with mounting holes."""
    edge_cuts = board.GetLayerID("Edge.Cuts")

    # Main outline (80mm x 60mm)
    pts = [
        pcbnew.VECTOR2I(-38000, -28000),
        pcbnew.VECTOR2I(38000, -28000),
        pcbnew.VECTOR2I(38000, 28000),
        pcbnew.VECTOR2I(-38000, 28000),
        pcbnew.VECTOR2I(-38000, -28000),
    ]
    for i in range(len(pts) - 1):
        seg = pcbnew.PCB_SHAPE(board)
        seg.SetShape(0)  # Segment
        seg.SetStart(pts[i])
        seg.SetEnd(pts[i + 1])
        seg.SetWidth(150000)
        seg.SetLayer(edge_cuts)
        board.Add(seg)

    # Mounting holes (4x M3, 3.2mm drill)
    mount_holes = [
        (-36000, -26000), (36000, -26000),
        (-36000, 26000), (36000, 26000),
    ]
    for x, y in mount_holes:
        via = pcbnew.PCB_VIA(board)
        via.SetPosition(pcbnew.VECTOR2I(x, y))
        via.SetWidth(int(2.0 * 1e6))
        via.SetDrill(int(3.2 * 1e6))
        via.SetViaType(pcbnew.VIATYPE_THROUGH)
        via.SetNetCode(0)  # GND
        via.SetLayerPair(board.GetLayerID("F.Cu"), board.GetLayerID("B.Cu"))
        board.Add(via)


def add_zone(board, layer, net_code, name, x1, y1, x2, y2):
    """Add a copper zone."""
    zone = pcbnew.ZONE(board)
    zone.SetLayer(layer)
    zone.SetNetCode(net_code)
    zone.SetZoneName(name)
    zone.SetMinThickness(int(0.254 * 1e6))
    zone.SetIsFilled(True)

    poly = pcbnew.SHAPE_POLY_SET()
    poly.NewOutline()
    poly.InsertVertex(0, pcbnew.VECTOR2I(int(x1), int(y1)))
    poly.InsertVertex(1, pcbnew.VECTOR2I(int(x2), int(y1)))
    poly.InsertVertex(2, pcbnew.VECTOR2I(int(x2), int(y2)))
    poly.InsertVertex(3, pcbnew.VECTOR2I(int(x1), int(y2)))
    zone.SetFilledPolysList(layer, poly)
    board.Add(zone)


def add_via(board, x, y, net_code=0):
    """Add a via."""
    via = pcbnew.PCB_VIA(board)
    via.SetPosition(pcbnew.VECTOR2I(x, y))
    via.SetWidth(int(0.8 * 1e6))
    via.SetDrill(int(0.4 * 1e6))
    via.SetViaType(pcbnew.VIATYPE_THROUGH)
    via.SetNetCode(net_code)
    via.SetLayerPair(board.GetLayerID("F.Cu"), board.GetLayerID("B.Cu"))
    board.Add(via)


def add_test_point(board, x, y, net_code, text):
    """Add a test point (circular via)."""
    via = pcbnew.PCB_VIA(board)
    via.SetPosition(pcbnew.VECTOR2I(int(x * 1e6), int(y * 1e6)))
    via.SetWidth(int(1.5 * 1e6))
    via.SetDrill(int(0.8 * 1e6))
    via.SetViaType(pcbnew.VIATYPE_THROUGH)
    via.SetNetCode(net_code)
    via.SetLayerPair(board.GetLayerID("F.Cu"), board.GetLayerID("B.Cu"))
    board.Add(via)


def main():
    """Generate complete PCB layout."""
    print("Creating board...")
    board = create_board()

    print("Adding edge cuts and mounting holes...")
    add_edge_cuts(board)

    print("Adding component footprints...")
    # Power connectors
    create_footprint(board, "USB-C-31-SR", "J1", "USB-C", -30, 25, [
        (1, -3.25, 2.5, 1.5, 1.2, 18), (2, -3.25, 0, 1.5, 1.2, 19),
        (3, -3.25, -2.5, 1.5, 1.2, 1), (4, -3.25, -5, 1.5, 1.2, 0),
    ])

    create_th_footprint(board, "TERM_BLOCK_2POS", "J2", "12V IN", -30, 25, [
        (1, -1.25, 0, 1.5, 3), (2, 1.25, 0, 1.5, 0),
    ])

    # JST-XH connectors
    create_footprint(board, "JST-XH-3", "J3", "BLUE WIRE", 30, 20, [
        (1, -1.25, 0, 1.5, 2.0, 4), (2, 0, 0, 1.5, 2.0, 5), (3, 1.25, 0, 1.5, 2.0, 0),
    ])

    create_footprint(board, "JST-XH-3", "J4", "DS18B20", 30, -20, [
        (1, -1.25, 0, 1.5, 2.0, 1), (2, 0, 0, 1.5, 2.0, 14), (3, 1.25, 0, 1.5, 2.0, 0),
    ])

    create_footprint(board, "JST-XH-4", "J5", "MQ-7", 30, -25, [
        (1, -1.875, 0, 1.5, 2.0, 2), (2, -0.625, 0, 1.5, 2.0, 0),
        (3, 0.625, 0, 1.5, 2.0, 15), (4, 1.875, 0, 1.5, 2.0, 16),
    ])

    # ESP32-S3 module (simplified - actual would need precise QFN footprint)
    esp32_x, esp32_y = 0, 0
    esp32_pads = []
    # Left side pins
    for i in range(14):
        y = -15.24 + i * 2.54
        esp32_pads.append((i + 1, -11, y, 1.5, 0.8, i + 1))
    # Right side pins
    for i in range(14):
        y = -15.24 + i * 2.54
        esp32_pads.append((i + 15, 11, y, 1.5, 0.8, i + 15))
    # Corner mounting pads
    esp32_pads.extend([
        (44, -14, -18, 2.5, 2.5, 0), (45, -14, 18, 2.5, 2.5, 0),
        (46, 14, -18, 2.5, 2.5, 0), (47, 14, 18, 2.5, 2.5, 0),
    ])
    esp32_fp = create_footprint(board, "ESP32-S3-WROOM-1-N8R8", "U1", "ESP32-S3-WROOM-1-N8R8",
                                esp32_x, esp32_y, esp32_pads)

    # Power regulators
    create_footprint(board, "MP2451", "U2", "MP2451", -20, -20, [
        (1, -1.9, -1.3, 1.0, 0.6, 3), (2, 0, -1.9, 1.0, 0.6, 0),
        (3, 1.9, -1.3, 1.0, 0.6, 4), (4, 1.9, 1.3, 1.0, 0.6, 1),
        (5, 0, 1.9, 1.0, 0.6, 1), (6, -1.9, 1.3, 1.0, 0.6, 1),
    ])

    create_footprint(board, "AP2112", "U3", "AP2112", -15, -20, [
        (1, -1.9, -1.3, 1.0, 0.6, 1), (2, 0, -1.9, 1.0, 0.6, 0),
        (3, 1.9, -1.3, 1.0, 0.6, 1), (4, 1.9, 1.3, 1.0, 0.6, 1),
        (5, 0, 1.9, 1.0, 0.6, 1),
    ])

    # Sensors
    create_footprint(board, "BME280", "U4", "BME280", -25, 10, [
        (1, -2, -2.5, 0.3, 1.2, 13), (2, -2, 0, 0.3, 1.2, 13),
        (3, -2, 2.5, 0.3, 1.2, 12), (4, 2, 2.5, 0.3, 1.2, 1),
        (5, 2, 0, 0.3, 1.2, 0), (6, 2, -2.5, 0.3, 1.2, 0),
    ])

    create_footprint(board, "DS3231", "U5", "DS3231", -25, -10, [
        (1, -2, -2.54, 0.6, 2.0, 12), (2, -2, 0, 0.6, 2.0, 0),
        (3, -2, 2.54, 0.6, 2.0, 13), (4, 2, 2.54, 0.6, 2.0, 12),
        (5, 2, 0, 0.6, 2.0, 0), (6, 2, -2.54, 0.6, 2.0, 13),
    ])

    # Touch controller
    create_footprint(board, "GT911", "U6", "GT911", 20, 15, [
        (1, -2, -2.5, 0.3, 1.2, 13), (2, -2, 0, 0.3, 1.2, 12),
        (3, -2, 2.5, 0.3, 1.2, 21), (4, 2, 2.5, 0.3, 1.2, 1),
        (5, 2, 0, 0.3, 1.2, 0), (6, 2, -2.5, 0.3, 1.2, 22),
    ])

    # Level shifters
    create_footprint(board, "BSS138", "U7", "BSS138", -20, 15, [
        (1, -1.9, -1.3, 1.0, 0.6, 4), (2, 0, -1.9, 1.0, 0.6, 0),
        (3, 1.9, -1.3, 1.0, 0.6, 4),
    ])

    create_footprint(board, "BSS138", "U8", "BSS138", -20, 10, [
        (1, -1.9, -1.3, 1.0, 0.6, 5), (2, 0, -1.9, 1.0, 0.6, 0),
        (3, 1.9, -1.3, 1.0, 0.6, 5),
    ])

    # 74LCX125 buffer
    create_footprint(board, "74LCX125", "U9", "74LCX125", -25, 20, [
        (1, -2.5, -2.5, 0.6, 2.0, 26), (2, -2.5, 0, 0.6, 2.0, 0),
        (3, -2.5, 2.5, 0.6, 2.0, 5), (4, 2.5, 2.5, 0.6, 2.0, 4),
        (5, 2.5, 0, 0.6, 2.0, 0), (6, 2.5, -2.5, 0.6, 2.0, 26),
    ])

    # Expansion headers
    h1_pads = []
    for i in range(10):
        y = -12.7 + i * 2.54
        h1_pads.append((i + 1, -2.54, y, 1.6, 1.6, i + 1))
        h1_pads.append((i + 11, 2.54, y, 1.6, 1.6, i + 11))
    create_footprint(board, "EXPANSION_2X10", "H1", "EXPANSION 1", -30, -15, h1_pads)

    h2_pads = []
    for i in range(10):
        y = -12.7 + i * 2.54
        h2_pads.append((i + 1, -2.54, y, 1.6, 1.6, i + 1))
        h2_pads.append((i + 11, 2.54, y, 1.6, 1.6, i + 11))
    create_footprint(board, "EXPANSION_2X10", "H2", "EXPANSION 2", -25, -15, h2_pads)

    # Passives (resistors, capacitors, LEDs)
    for i, (x, y, ref) in enumerate([
        (-25, 0, "R1"), (-23, 0, "R2"), (-21, 0, "R3"), (-19, 0, "R4"),
        (-17, 0, "R5"), (-15, 0, "R6"), (-13, 0, "R7"), (-11, 0, "R8"),
    ]):
        create_footprint(board, "RES", ref, "10k", x, y, [
            (1, -0.5, 0, 0.6, 0.6, i + 1), (2, 0.5, 0, 0.6, 0.6, i + 11),
        ])

    for i, (x, y, ref) in enumerate([
        (-25, -5, "C1"), (-23, -5, "C2"), (-21, -5, "C3"), (-19, -5, "C4"),
        (-17, -5, "C5"), (-15, -5, "C6"), (-13, -5, "C7"), (-11, -5, "C8"),
    ]):
        create_footprint(board, "CAP", ref, "10uF", x, y, [
            (1, -0.5, 0, 0.6, 0.6, i + 1), (2, 0.5, 0, 0.6, 0.6, i + 11),
        ])

    # LEDs
    create_footprint(board, "LED", "D1", "LED1", -28, -10, [
        (1, -0.5, 0, 0.6, 0.6, 0), (2, 0.5, 0, 0.6, 0.6, 26),
    ])
    create_footprint(board, "LED", "D2", "LED2", -26, -10, [
        (1, -0.5, 0, 0.6, 0.6, 0), (2, 0.5, 0, 0.6, 0.6, 27),
    ])

    # Buttons
    create_th_footprint(board, "SW_TACTILE", "SW1", "BUTTON1", -28, -15, [
        (1, -1.5, -1.5, 0.8, 26), (2, 1.5, -1.5, 0.8, 26),
        (3, -1.5, 1.5, 0.8, 26), (4, 1.5, 1.5, 0.8, 26),
    ])
    create_th_footprint(board, "SW_TACTILE", "SW2", "BUTTON2", -26, -15, [
        (1, -1.5, -1.5, 0.8, 27), (2, 1.5, -1.5, 0.8, 27),
        (3, -1.5, 1.5, 0.8, 27), (4, 1.5, 1.5, 0.8, 27),
    ])

    # Crystal
    create_footprint(board, "CRYSTAL_2PIN", "Y1", "26MHz", -20, -25, [
        (1, -0.5, 0, 0.6, 0.6, 19), (2, 0.5, 0, 0.6, 0.6, 20),
    ])

    print("Adding copper zones...")
    # Ground planes
    add_zone(board, board.GetLayerID("B.Cu"), 0, "GND_Bottom",
             -40000, -30000, 40000, 30000)
    add_zone(board, board.GetLayerID("F.Cu"), 0, "GND_Top",
             -40000, -30000, 40000, 30000)

    # Power zones
    add_zone(board, board.GetLayerID("F.Cu"), 1, "3V3_Top",
             15000, -10000, 35000, 10000)
    add_zone(board, board.GetLayerID("F.Cu"), 2, "5V_Top",
             15000, 15000, 35000, 25000)
    add_zone(board, board.GetLayerID("F.Cu"), 3, "12V_Top",
             15000, -25000, 35000, -15000)

    print("Adding ground vias...")
    for x in range(-35, 36, 10):
        for y in range(-25, 26, 10):
            add_via(board, x, y, 0)

    print("Adding test points...")
    add_test_point(board, -35, 25, 0, "TP_GND")
    add_test_point(board, -30, 25, 1, "TP_3V3")
    add_test_point(board, -25, 25, 2, "TP_5V")
    add_test_point(board, -20, 25, 3, "TP_12V")

    print("Adding silkscreen...")
    # Board name
    txt = pcbnew.PCB_SHAPE(board)
    txt.SetShape(0)
    txt.SetStart(pcbnew.VECTOR2I(-15000, 27000))
    txt.SetEnd(pcbnew.VECTOR2I(15000, 27000))
    txt.SetWidth(150000)
    txt.SetLayer(board.GetLayerID("F.SilkS"))
    board.Add(txt)

    # Revision
    txt2 = pcbnew.PCB_SHAPE(board)
    txt2.SetShape(0)
    txt2.SetStart(pcbnew.VECTOR2I(-10000, 26500))
    txt2.SetEnd(pcbnew.VECTOR2I(10000, 26500))
    txt2.SetWidth(150000)
    txt2.SetLayer(board.GetLayerID("F.SilkS"))
    board.Add(txt2)

    print("Saving board...")
    board.Save(str(PCB_FILE))
    print(f"Board saved to: {PCB_FILE}")


if __name__ == "__main__":
    main()
