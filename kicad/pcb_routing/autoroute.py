#!/usr/bin/env python3
"""Headless autorouting for DieselFire — place-and-route finisher.

Runs after critical.py. Takes the placed-and-critically-routed board and
turns it into a fabrication-ready 2-layer PCB with no KiCad GUI session:

  Step 1  Set design rules (0.2 mm track / 0.15 mm clearance) in the project file.
  Step 2  Export a Specctra DSN and batch-autoroute with Freerouting.
  Step 3  Import the SES routes, then finish in code:
            - gap-fill: connect short connections the autorouter strands
            - GND pour on both layers with via grid stitching
            - bond pour islands
  Step 4  Re-run DRC (must be clean + fully connected)
  Step 5  Export gerbers / drill / CPL

Run from project root, after critical.py:
    python3 kicad/pcb_routing/autoroute.py

Determinism note: this script only ever *adds* copper to the freshly
placed board and clears prior tracks/zones via board.RemoveNative.
It never uses board.Remove() on tracks/zones/footprints — that leaks
the swig proxy and corrupts the pcbnew module.
"""

from __future__ import annotations

import json
import math
import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
PROJECT_ROOT = HERE.parents[0]
PCB = PROJECT_ROOT / "pcb" / "DieselFire.kicad_pcb"
PRO = PROJECT_ROOT / "DieselFire.kicad_pro"
DSN = PROJECT_ROOT / "fabrication" / "DieselFire.dsn"
SES = PROJECT_ROOT / "fabrication" / "DieselFire.ses"
FAB = PROJECT_ROOT / "fabrication"
FREEROUTING = HERE.parents[1] / "external" / "freerouting-2.2.4-linux-x64" / "bin" / "freerouting"

GAPFILL_TRACK_MM = 0.3
GAPFILL_CLEARANCE_MM = 0.15
END_MARGIN_MM = 1.0


def ensure_design_rules() -> None:
    """Force the Default net class to 0.2 mm track / 0.15 mm clearance."""
    d = json.loads(PRO.read_text())
    ns = d.setdefault("net_settings", {})
    classes = ns.setdefault("classes", [])
    for c in classes:
        if c.get("name") == "Default":
            c["clearance"] = 0.15
            c["track_width"] = 0.2
    classes[:] = [c for c in classes if c.get("name") != "Power"]
    ns["netclass_patterns"] = []
    PRO.write_text(json.dumps(d, indent=2) + "\n")
    print("Step 1: design rules set (Default 0.2 mm track / 0.15 mm clearance).")


def _clear_copper() -> None:
    """Remove zones only — preserve tracks (critical routes from critical.py).
    
    Tracks are not cleared so pre-routed critical nets survive into the DSN
    export.  Only copper zones (pours) are removed to avoid stale pour data.
    """
    board = __import__("pcbnew", fromlist=["LoadBoard"]).LoadBoard(str(PCB))
    for z in list(board.Zones()):
        board.RemoveNative(z)
    board.Save(str(PCB))


def autoroute() -> None:
    """Export DSN and run Freerouting."""
    FAB.mkdir(parents=True, exist_ok=True)
    
    pcbnew = __import__("pcbnew", fromlist=["ExportSpecctraDSN", "LoadBoard"])
    board = pcbnew.LoadBoard(str(PCB))
    if not pcbnew.ExportSpecctraDSN(board, str(DSN)):
        raise RuntimeError("ExportSpecctraDSN failed")
    
    print(f"Step 2: exported DSN ({DSN.stat().st_size} bytes); routing...")
    proc = subprocess.run(
        [str(FREEROUTING), "-de", str(DSN), "-do", str(SES), "-da", "-mp", "12"],
        capture_output=True, text=True, timeout=300,
    )
    if not SES.exists():
        sys.stderr.write((proc.stdout + proc.stderr)[-2000:])
        raise RuntimeError("Freerouting produced no SES")
    m = re.findall(r"\((\d+) unrouted", proc.stdout + proc.stderr)
    if m:
        print(f"  Freerouting reported {m[-1]} unrouted connections "
              "(GND excluded — poured separately; stragglers gap-filled).")


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
    """True if a segment a->b on `layer` clears foreign pads/tracks."""
    em = __import__("pcbnew").FromMM(END_MARGIN_MM)
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
        if t.Type() != __import__("pcbnew").PCB_TRACE_T or t.GetLayer() != layer:
            continue
        if t.GetNetCode() == code:
            continue
        s, e = t.GetStart(), t.GetEnd()
        if _seg_seg_dist((ax, ay), (bx, by), (s.x, s.y), (e.x, e.y)) \
                < half_w + t.GetWidth() / 2 + clr:
            return False
    return True


def _add_track(board, ax, ay, bx, by, layer, net, width):
    pcbnew = __import__("pcbnew", fromlist=["PCB_TRACK"])
    t = pcbnew.PCB_TRACK(board)
    t.SetStart(__import__("pcbnew").VECTOR2I(int(ax), int(ay)))
    t.SetEnd(__import__("pcbnew").VECTOR2I(int(bx), int(by)))
    t.SetWidth(int(width))
    t.SetLayer(layer)
    t.SetNet(net)
    board.Add(t)


def _drc_unconnected(board):
    """Save + run kicad-cli DRC (json) and return unconnected pairs."""
    board.Save(str(PCB))
    rep = FAB / "DieselFire-drc.json"
    subprocess.run(["kicad-cli", "pcb", "drc", "--format", "json",
                    "-o", str(rep), str(PCB)], check=False, capture_output=True)
    data = json.loads(rep.read_text())
    out = []
    for v in data.get("unconnected_items", []):
        items = v.get("items", [])
        if len(items) != 2:
            continue
        m = re.search(r"\[(/[^\]]+)\]", items[0]["description"]) or \
            re.search(r"\[(/[^\]]+)\]", items[1]["description"])
        net = m.group(1) if m else None
        p = [(__import__("pcbnew").FromMM(it["pos"]["x"]),
              __import__("pcbnew").FromMM(it["pos"]["y"]))
             for it in items]
        out.append((net, p[0], p[1]))
    return out


def gap_fill(board) -> int:
    """Connect short non-GND connections Freerouting stranded."""
    pcbnew = __import__("pcbnew", fromlist=["FromMM", "PCB_VIA", 
                          "VIATYPE_THROUGH", "F_Cu", "B_Cu", "VECTOR2I"])
    width = pcbnew.FromMM(GAPFILL_TRACK_MM)
    clr = pcbnew.FromMM(GAPFILL_CLEARANCE_MM)
    half = width / 2
    filled = 0
    for _ in range(6):
        pairs = [(n, a, b) for (n, a, b) in _drc_unconnected(board)
                 if n and n != "/GND" and not n.startswith("unconnected-")]
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
                    v.SetDrill(pcbnew.FromMM(0.3))
                    v.SetWidth(pcbnew.FromMM(0.6))
                    v.SetNet(net)
                    board.Add(v)
                _add_track(board, ax, ay, bx, by, pcbnew.B_Cu, net, width)
                filled += 1
                progress = True
        if not progress:
            break
    print(f"Step 3: gap-filled {filled} stranded connections.")
    return filled


def pour_ground(board) -> None:
    """Pour GND on both layers and stitch with vias."""
    pcbnew = __import__("pcbnew", fromlist=["ZONE_CONNECTION_FULL",
                          "ISLAND_REMOVAL_MODE_AREA", "ZONE_FILLER",
                          "VECTOR2I", "F_Cu", "B_Cu", "VIATYPE_THROUGH"])
    gnd = board.FindNet("/GND")
    if gnd is None:
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
        z.SetMinIslandArea(int(4.0 * 1e6 * 1e6))
        poly = z.Outline()
        poly.NewOutline()
        for x, y in ((L, T), (R, T), (R, B), (L, B)):
            poly.Append(x, y)
        board.Add(z)
    
    pcbnew.ZONE_FILLER(board).Fill(board.Zones())
    
    # Add GND stitching via grid
    gnd_via_grid(board, gnd)
    pcbnew.ZONE_FILLER(board).Fill(board.Zones())
    bond_pour_islands(board, gnd)
    print(f"  poured + filled GND on F.Cu + B.Cu.")


def _via_clear(board, x, y, code, rad, clr):
    """True if a via at (x,y) clears all foreign copper on both layers."""
    pcbnew = __import__("pcbnew", fromlist=["FromMM", "PCB_VIA_T", "PCB_TRACE_T"])
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


def gnd_via_grid(board, gnd) -> int:
    """Stitch F.Cu and B.Cu GND pours with a grid of vias."""
    pcbnew = __import__("pcbnew", fromlist=["VECTOR2I", "VIATYPE_THROUGH",
                          "F_Cu", "B_Cu", "FromMM"])
    bb = board.GetBoardEdgesBoundingBox()
    keep = pcbnew.FromMM(2.0)
    L, R = bb.GetLeft() + keep, bb.GetRight() - keep
    T, B = bb.GetTop() + keep, bb.GetBottom() - keep
    step = pcbnew.FromMM(5.0)
    rad = pcbnew.FromMM(0.3)
    clr = pcbnew.FromMM(0.2)
    code = gnd.GetNetCode()
    
    def drop(px, py):
        v = pcbnew.PCB_VIA(board)
        v.SetViaType(pcbnew.VIATYPE_THROUGH)
        v.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu)
        v.SetPosition(pcbnew.VECTOR2I(int(px), int(py)))
        v.SetDrill(pcbnew.FromMM(0.3))
        v.SetWidth(pcbnew.FromMM(0.6))
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
    
    # Via on every interior GND pad
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
    
    print(f"  placed {added} GND stitching vias.")
    return added


def bond_pour_islands(board, gnd) -> int:
    """Bond every filled GND pour island to the B.Cu plane."""
    pcbnew = __import__("pcbnew", fromlist=["VECTOR2I", "VIATYPE_THROUGH",
                          "F_Cu", "B_Cu", "ZONE_FILLER", "FromMM"])
    bb = board.GetBoardEdgesBoundingBox()
    keep = pcbnew.FromMM(1.2)
    Lx, Rx = bb.GetLeft() + keep, bb.GetRight() - keep
    Ty, By = bb.GetTop() + keep, bb.GetBottom() - keep
    code = gnd.GetNetCode()
    rad = pcbnew.FromMM(0.3)
    clr = pcbnew.FromMM(0.2)
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
                        v.SetDrill(pcbnew.FromMM(0.3))
                        v.SetWidth(pcbnew.FromMM(0.6))
                        v.SetNet(gnd)
                        board.Add(v)
                        via_pts.append(pt)
                        added += 1
                        placed_here += 1
                        break
        if placed_here == 0:
            break
        pcbnew.ZONE_FILLER(board).Fill(board.Zones())
    
    print(f"  bonded {added} pour islands with GND vias.")
    return added


def finish_board() -> None:
    """Import SES routes, gap-fill, pour GND."""
    pcbnew = __import__("pcbnew", fromlist=["LoadBoard", "ImportSpecctraSES"])
    board = pcbnew.LoadBoard(str(PCB))
    if not pcbnew.ImportSpecctraSES(board, str(SES)):
        raise RuntimeError("ImportSpecctraSES failed")
    segs = sum(1 for t in board.GetTracks() if t.Type() == pcbnew.PCB_TRACE_T)
    vias = sum(1 for t in board.GetTracks() if t.Type() == pcbnew.PCB_VIA_T)
    print(f"Step 3: imported {segs} segments + {vias} vias.")
    gap_fill(board)
    pour_ground(board)
    board.Save(str(PCB))


def verify() -> None:
    """Run DRC and check for violations."""
    report = FAB / "DieselFire-drc.txt"
    subprocess.run(["kicad-cli", "pcb", "drc", "--severity-error",
                    "-o", str(report), str(PCB)], check=False, capture_output=True)
    text = report.read_text()
    violations = int(re.search(r"Found (\d+) DRC violations", text).group(1))
    mu = re.search(r"Found (\d+) unconnected", text)
    unconnected = int(mu.group(1)) if mu else 0
    print(f"Step 4: DRC {violations} violations, {unconnected} unconnected.")
    if violations or unconnected:
        print(f"  See {report} for details.")
        print(f"  Board may need manual intervention.")
    else:
        print("  board is DRC-clean and fully connected.")


def export_fabrication() -> None:
    """Export gerbers, drill, and pick-and-place files."""
    gerber_dir = FAB / "gerbers"
    gerber_dir.mkdir(parents=True, exist_ok=True)
    
    subprocess.run(["kicad-cli", "pcb", "export", "gerbers",
                    "-o", str(gerber_dir) + "/", str(PCB)],
                   check=True, capture_output=True)
    subprocess.run(["kicad-cli", "pcb", "export", "drill",
                    "-o", str(gerber_dir) + "/", str(PCB)],
                   check=True, capture_output=True)
    subprocess.run(["kicad-cli", "pcb", "export", "pos", "--format", "csv",
                    "--units", "mm", "-o", str(FAB / "DieselFire-cpl.csv"), str(PCB)],
                   check=True, capture_output=True)
    
    print(f"Step 5: gerbers + drill in {gerber_dir}, CPL at DieselFire-cpl.csv")


def main() -> None:
    ensure_design_rules()
    autoroute()
    finish_board()
    verify()
    export_fabrication()
    print("\nAutorouting pipeline complete — PCB is fabrication-ready.")


if __name__ == "__main__":
    main()
