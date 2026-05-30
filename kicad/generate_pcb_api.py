#!/usr/bin/env python3
"""
Generate KiCad 9.0 PCB layout for DieselFire S3 using the KiCad Python API.
This ensures proper format compliance.
"""

import sys
sys.path.insert(0, '/usr/lib/python3/dist-packages')
import pcbnew
from pathlib import Path

BASE = Path(__file__).parent
PCB_FILE = BASE / "pcb" / "Afterburner-Modern.kicad_pcb"


def create_board():
    """Create a new KiCad board with proper settings."""
    board = pcbnew.BOARD()
    board.SetCopperLayerCount(2)
    board.m_Size = pcbnew.VECTOR2I(80000, 60000)  # 80mm x 60mm

    tb = board.GetTitleBlock()
    tb.SetTitle("DieselFire S3")
    tb.SetDate("2026-05-29")
    tb.SetRevision("1.0")
    tb.SetCompany("Afterburner")

    # Set up net classes
    net_class = pcbnew.NETCLASS(board)
    net_class.SetName("Default")
    net_class.SetClearance(int(0.2 * 1e6))  # Convert to internal units
    net_class.SetTrackWidth(int(0.25 * 1e6))
    net_class.SetViaDiameter(int(0.8 * 1e6))
    net_class.SetViaDrill(int(0.4 * 1e6))
    net_class.SetuViaDiameter(int(0.3 * 1e6))
    net_class.SetuViaDrill(int(0.1 * 1e6))
    board.GetNetClasses()["Default"] = net_class

    return board


def add_edge_cuts(board):
    """Add board outline (80mm x 60mm rectangle)."""
    edge_cuts = board.GetLayerID("Edge.Cuts")
    pts = [
        pcbnew.VECTOR2I(-40000, -30000),
        pcbnew.VECTOR2I(40000, -30000),
        pcbnew.VECTOR2I(40000, 30000),
        pcbnew.VECTOR2I(-40000, 30000),
        pcbnew.VECTOR2I(-40000, -30000),
    ]
    for i in range(len(pts) - 1):
        seg = pcbnew.PCB_SHAPE(board)
        seg.SetShape(0)  # Segment
        seg.SetStart(pts[i])
        seg.SetEnd(pts[i + 1])
        seg.SetWidth(150000)
        seg.SetLayer(edge_cuts)
        board.Add(seg)


def add_ground_zone(board, layer, x1, y1, x2, y2):
    """Add a filled copper zone."""
    zone = pcbnew.ZONE(board)
    zone.SetLayer(layer)
    zone.SetNetCode(0)  # GND
    zone.SetZoneName("GND")
    zone.SetMinThickness(int(0.254 * 1e6))  # 0.254mm in internal units
    zone.SetIsFilled(True)

    # Create polygon
    poly = pcbnew.SHAPE_POLY_SET()
    poly.NewOutline()
    poly.InsertVertex(0, pcbnew.VECTOR2I(x1, y1))
    poly.InsertVertex(1, pcbnew.VECTOR2I(x2, y1))
    poly.InsertVertex(2, pcbnew.VECTOR2I(x2, y2))
    poly.InsertVertex(3, pcbnew.VECTOR2I(x1, y2))
    zone.SetFilledPolysList(layer, poly)
    board.Add(zone)


def add_power_zone(board, layer, net_code, name, x1, y1, x2, y2):
    """Add a filled copper zone for power nets."""
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
    via.SetWidth(int(1.2 * 1e6))  # 1.2mm
    via.SetDrill(int(0.6 * 1e6))  # 0.6mm
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


def add_silk_text(board, text, x, y, layer, size=800000):
    """Add silkscreen text."""
    txt = pcbnew.PCB_SHAPE(board)
    txt.SetShape(0)  # This won't work for text - we need a different approach
    # Actually, let's skip manual silkscreen text for now


def main():
    """Generate the complete PCB."""
    print("Creating KiCad board...")
    board = create_board()

    print("Adding edge cuts...")
    add_edge_cuts(board)

    print("Adding ground zones...")
    # Bottom layer GND plane
    add_ground_zone(board, board.GetLayerID("B.Cu"), -40000, -30000, 40000, 30000)
    # Top layer GND plane
    add_ground_zone(board, board.GetLayerID("F.Cu"), -40000, -30000, 40000, 30000)

    print("Adding power zones...")
    # 3.3V zone
    add_power_zone(board, board.GetLayerID("F.Cu"), 1, "3V3",
                   -35, -25, -15, 15)
    # 5V zone
    add_power_zone(board, board.GetLayerID("F.Cu"), 2, "5V",
                   -10, -25, 10, 25)
    # 12V zone
    add_power_zone(board, board.GetLayerID("F.Cu"), 3, "12V",
                   15, -25, 30, 25)

    print("Adding vias...")
    # Via grid for ground connections
    for x in range(-35, 36, 10):
        for y in range(-25, 26, 10):
            add_via(board, x, y, 0)

    print("Adding tracks...")
    # 12V input traces
    add_track(board, -30, -20, -20, -20, 1.0, "F.Cu", 3)
    add_track(board, -20, -20, -20, -10, 1.0, "F.Cu", 3)
    # 5V traces
    add_track(board, -15, -10, -5, -10, 0.5, "F.Cu", 2)
    add_track(board, -5, -10, -5, 0, 0.5, "F.Cu", 2)
    # 3.3V traces
    add_track(board, 0, 0, 10, 0, 0.3, "F.Cu", 1)
    add_track(board, 10, 0, 10, 10, 0.3, "F.Cu", 1)
    # UART traces
    add_track(board, -25, 15, -30, 15, 0.3, "F.Cu", 4)
    add_track(board, -30, 15, -30, 5, 0.3, "F.Cu", 4)
    # SPI traces
    add_track(board, 5, 5, 15, 5, 0.2, "F.Cu", 5)
    add_track(board, 5, 10, 15, 10, 0.2, "F.Cu", 6)
    add_track(board, 5, 15, 15, 15, 0.2, "F.Cu", 7)
    # I2C traces
    add_track(board, 5, -5, 15, -5, 0.2, "F.Cu", 8)
    add_track(board, 5, -10, 15, -10, 0.2, "F.Cu", 9)
    # OneWire trace
    add_track(board, 20, -15, 30, -15, 0.2, "F.Cu", 10)
    # ADC trace
    add_track(board, 20, -20, 25, -20, 0.3, "F.Cu", 11)

    print("Saving board...")
    board.Save(str(PCB_FILE))
    print(f"Board saved to: {PCB_FILE}")


if __name__ == "__main__":
    main()
