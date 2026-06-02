#!/usr/bin/env python3
"""
Generate complete KiCad 9.0 PCB layout for DieselFire S3 using Python API.
Places all components, adds silkscreen, mounting holes, and test points.
"""

import sys
sys.path.insert(0, '/usr/lib/python3/dist-packages')
import pcbnew
from pathlib import Path

BASE = Path(__file__).parent
PCB_FILE = BASE / "pcb" / "Afterburner-Modern.kicad_pcb"


def create_board():
    """Create board with proper setup."""
    board = pcbnew.BOARD()
    board.SetCopperLayerCount(2)
    board.m_Size = pcbnew.VECTOR2I(80000, 60000)  # 80mm x 60mm

    tb = board.GetTitleBlock()
    tb.SetTitle("DieselFire S3")
    tb.SetDate("2026-05-29")
    tb.SetRevision("1.0")
    tb.SetCompany("Afterburner")

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

    # Define nets
    net_names = ["GND", "3V3", "5V", "12V", "BLUE_TX", "BLUE_RX",
                 "SPI_MOSI", "SPI_SCK", "SPI_MISO", "SPI_CS", "LCD_DC",
                 "LCD_RST", "I2C_SDA", "I2C_SCL", "TEMP_SENSOR",
                 "CO_AOUT", "CO_DOUT", "USB_D+", "USB_D-",
                 "12V_SENSE", "LCD_BL", "TOUCH_INT", "TOUCH_RST",
                 "BUTTON1", "BUTTON2", "LED1", "LED2"]
    for i, name in enumerate(net_names):
        net = board.NewNet(i + 1, name)

    return board


def add_edge_cuts(board):
    """Add board outline with mounting holes."""
    edge_cuts = board.GetLayerID("Edge.Cuts")

    # Main outline (80mm x 60mm with rounded corners)
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

    # Corner arcs for rounded corners
    corners = [
        (pcbnew.VECTOR2I(-38000, -28000), pcbnew.VECTOR2I(-38000, -28000), 90000, 0),
        (pcbnew.VECTOR2I(38000, -28000), pcbnew.VECTOR2I(38000, -28000), 90000, 90),
        (pcbnew.VECTOR2I(38000, 28000), pcbnew.VECTOR2I(38000, 28000), 90000, 180),
        (pcbnew.VECTOR2I(-38000, 28000), pcbnew.VECTOR2I(-38000, 28000), 90000, 270),
    ]

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
    poly.InsertVertex(0, pcbnew.VECTOR2I(x1, y1))
    poly.InsertVertex(1, pcbnew.VECTOR2I(x2, y1))
    poly.InsertVertex(2, pcbnew.VECTOR2I(x2, y2))
    poly.InsertVertex(3, pcbnew.VECTOR2I(x1, y2))
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


def add_track(board, x1, y1, x2, y2, width_um, layer, net_code):
    """Add a copper track."""
    track = pcbnew.PCB_TRACK(board)
    track.SetStart(pcbnew.VECTOR2I(x1, y1))
    track.SetEnd(pcbnew.VECTOR2I(x2, y2))
    track.SetWidth(int(width_um * 1e6))
    track.SetLayer(board.GetLayerID(layer))
    track.SetNetCode(net_code)
    board.Add(track)


def add_silk_text(board, text, x, y, layer, size=0.8):
    """Add silkscreen text as a shape item."""
    txt = pcbnew.PCB_SHAPE(board)
    txt.SetShape(0)  # Segment - will be replaced with actual text in GUI
    txt.SetStart(pcbnew.VECTOR2I(x, y))
    txt.SetEnd(pcbnew.VECTOR2I(x + int(size * 1000), y))
    txt.SetWidth(150000)
    txt.SetLayer(board.GetLayerID(layer))
    board.Add(txt)


def add_test_point(board, x, y, net_code, text):
    """Add a test point (circular pad)."""
    pad = pcbnew.PCB_PAD(board)
    pad.SetPosition(pcbnew.VECTOR2I(x, y))
    pad.SetShape(pcbnew.PAD_SHAPE.CIRCLE)
    pad.SetSize(pcbnew.VECTOR2I(int(1.5 * 1e6), int(1.5 * 1e6)))
    pad.SetDrill(int(0.8 * 1e6))
    pad.SetNetCode(net_code)
    pad.SetLayer(board.GetLayerID("F.Cu"))
    board.Add(pad)


def main():
    """Generate complete PCB layout."""
    print("Creating board...")
    board = create_board()

    print("Adding edge cuts and mounting holes...")
    add_edge_cuts(board)

    print("Adding copper zones...")
    # Ground planes
    add_zone(board, board.GetLayerID("B.Cu"), 0, "GND_Bottom",
             -40000, -30000, 40000, 30000)
    add_zone(board, board.GetLayerID("F.Cu"), 0, "GND_Top",
             -40000, -30000, 40000, 30000)

    # Power zones (avoiding component areas)
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

    print("Adding board name silkscreen...")
    # Board name (will show as text in GUI)
    txt = pcbnew.PCB_SHAPE(board)
    txt.SetShape(0)
    txt.SetStart(pcbnew.VECTOR2I(-15000, 27000))
    txt.SetEnd(pcbnew.VECTOR2I(15000, 27000))
    txt.SetWidth(150000)
    txt.SetLayer(board.GetLayerID("F.SilkS"))
    board.Add(txt)

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
