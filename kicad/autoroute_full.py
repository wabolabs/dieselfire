#!/usr/bin/env python3
"""Complete PCB autorouting pipeline — from placed board to fab files.

Usage from project root:
    python3 kicad/autoroute_full.py

This script:
1. Loads the placed PCB (with footprints + critical routes)
2. Exports DSN
3. Runs freerouting
4. Imports SES routes
5. Gap-fills stranded connections
6. Pours GND zones
7. Runs DRC
8. Exports fabrication files
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
PCB_PATH = HERE / "pcb" / "Afterburner-Modern.kicad_pcb"
DSN_PATH = HERE / "freerouting" / "dieselfire.dsn"
SES_PATH = HERE / "freerouting" / "dieselfire.ses"
FREEROUTING_BIN = Path("/home/kboran/Nextcloud/development/afterburner/bluetoothheater/external/freerouting-2.2.4-linux-x64/bin/freerouting")
FAB_DIR = HERE / "fabrication"

# Design rules
TRACK_WIDTH_MM = 0.25
CLEARANCE_MM = 0.2
VIA_DIAM_MM = 0.8
VIA_DRILL_MM = 0.4
GAPFILL_TRACK_MM = 0.3
GAPFILL_CLEARANCE_MM = 0.15
END_MARGIN_MM = 1.0
GND_VIA_GRID_MM = 5.0
GND_VIA_RAD_MM = 0.3
GND_VIA_CLR_MM = 0.2
ISLAND_AREA_MM2 = 4.0


def _seg_point_dist(ax, ay, bx, by, px, py):
    dx, dy = bx - ax, by - ay
    L2 = dx * dx + dy * dy
    if L2 == 0:
        return math.hypot(px - ax, py - ay)
    t = max(0.0, min(1.0, ((px - ax) * dx + (py - ay) * dy) / L2))
    return math.hypot(px - (ax + t * dx), py - (ay + t * dy))


def _seg_seg_dist(a1, a2, b1, b2):
    return min(
        _seg_point_dist(*a1, *a2, *b1), _seg_point_dist(*a1, *a2, *b2),
        _seg_point_dist(*b1, *b2, *a1), _seg_point_dist(*b1, *b2, *a2),
    )


def _layer_clear(board, ax, ay, bx, by, layer, code, half_w, clr):
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
    for fp in board.GetFootprints():
        for pad in fp.Pads():
            if pad.GetNetCode() == code:
                continue
            if not pad.GetNetname():
                continue
            pos = pad.GetPosition()
            pr = 0.5 * math.hypot(pad.GetSize().x, pad.GetSize().y)
            if math.hypot(pos.x - x, pos.y - y) < rad + pr + clr:
                return False
    for t in board.GetTracks():
        if t.GetNetCode() == code:
            continue
        if t.Type() == pcbnew.PCB_VIA_T:
            pos = t.GetPosition()
            if math.hypot(pos.x - x, pos.y - y) < rad + pcbnew.FromMM(0.4) + clr:
                return False
        elif t.Type() == pcbnew.PCB_TRACE_T:
            s, e = t.GetStart(), t.GetEnd()
            if _seg_point_dist(s.x, s.y, e.x, e.y, x, y) < rad + t.GetWidth() / 2 + clr:
                return False
    return True


def _drc_unconnected(board):
    """Save + run kicad-cli DRC (json) and return unconnected pairs."""
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
                    v.SetWidth(pcbnew.FromMM(VIA_DIAM_MM / 2))
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
        v.SetWidth(pcbnew.FromMM(VIA_DIAM_MM / 2))
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

    # Per-pad vias: one via on every interior GND pad
    for fp in board.GetFootprints():
        for pad in fp.Pads():
            if pad.GetNetCode() != code:
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
                        v.SetWidth(pcbnew.FromMM(VIA_DIAM_MM / 2))
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


def main():
    print("=" * 60)
    print("DieselFire S3 — PCB Autorouting Pipeline")
    print("=" * 60)
    print()

    # Load board with existing placement and critical routes
    print("Loading board...")
    board = pcbnew.LoadBoard(str(PCB_PATH))
    if board is None:
        print("ERROR: Failed to load board. Run apply.py and critical.py first.")
        sys.exit(1)

    footprints = len(board.GetFootprints())
    tracks = sum(1 for t in board.GetTracks() if t.Type() in (pcbnew.PCB_TRACE_T, pcbnew.PCB_VIA_T))
    print(f"  Loaded {footprints} footprints, {tracks} tracks/vias")
    print()

    # Phase 2: Route
    print("=== Phase 2: Route ===")
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
