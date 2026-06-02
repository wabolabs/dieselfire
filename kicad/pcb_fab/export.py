#!/usr/bin/env python3
"""Export DieselFire fabrication artifacts (gerbers, drill, pick-and-place).

Wraps kicad-cli to produce everything JLCPCB needs. Outputs land in
kicad/fabrication/ in the canonical layout that JLCPCB and other
manufacturers expect:

    fabrication/
      gerbers/
        DieselFire-F_Cu.gbr
        DieselFire-B_Cu.gbr
        DieselFire-F_Paste.gbr
        DieselFire-B_Paste.gbr
        DieselFire-F_Silkscreen.gbr
        DieselFire-B_Silkscreen.gbr
        DieselFire-F_Mask.gbr
        DieselFire-B_Mask.gbr
        DieselFire-Edge_Cuts.gbr
      drill/
        DieselFire-PTH.drl
        DieselFire-NPTH.drl
      pick-and-place.csv
      drc-report.txt

Run from project root:
    python3 kicad/pcb_fab/export.py
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[1]  # kicad/
PCB = PROJECT_ROOT / "pcb" / "DieselFire.kicad_pcb"
FAB = PROJECT_ROOT / "fabrication"

GERBER_LAYERS = [
    "F.Cu", "B.Cu",
    "F.Paste", "B.Paste",
    "F.Silkscreen", "B.Silkscreen",
    "F.Mask", "B.Mask",
    "Edge.Cuts",
]


def _run(cmd: list[str]) -> None:
    print("  →", " ".join(str(c) for c in cmd))
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        sys.stderr.write(result.stdout + result.stderr)
        raise SystemExit(result.returncode)


def run_drc() -> None:
    """Run DRC and report results."""
    print("Running DRC...")
    drc_out = FAB / "drc-report.txt"
    FAB.mkdir(parents=True, exist_ok=True)
    result = subprocess.run(
        ["kicad-cli", "pcb", "drc", "--severity-all",
         "--output", str(drc_out), str(PCB)],
        capture_output=True, text=True,
    )
    output = result.stdout.strip()
    print(output)
    
    violations = 0
    unconnected = 0
    m_v = __import__("re").search(r"Found (\d+) DRC violations", output)
    m_u = __import__("re").search(r"Found (\d+) unconnected", output)
    if m_v:
        violations = int(m_v.group(1))
    if m_u:
        unconnected = int(m_u.group(1))
    
    if violations or unconnected:
        print(f"WARNING: {violations} violations, {unconnected} unconnected "
              f"(see {drc_out})")
    else:
        print("DRC clean — no violations or unconnected nets.")


def export_gerbers() -> None:
    """Export Gerber files for all layers."""
    out_dir = FAB / "gerbers"
    out_dir.mkdir(parents=True, exist_ok=True)
    _run([
        "kicad-cli", "pcb", "export", "gerbers",
        "--output", str(out_dir),
        "--layers", ",".join(GERBER_LAYERS),
        "--no-x2",
        "--subtract-soldermask",
        str(PCB),
    ])
    print(f"  Gerbers exported to {out_dir}/")


def export_drill() -> None:
    """Export Excellon drill files."""
    out_dir = FAB / "drill"
    out_dir.mkdir(parents=True, exist_ok=True)
    _run([
        "kicad-cli", "pcb", "export", "drill",
        "--output", str(out_dir) + "/",
        "--format", "excellon",
        "--drill-origin", "absolute",
        "--excellon-zeros-format", "decimal",
        "--excellon-units", "mm",
        "--excellon-separate-th",
        str(PCB),
    ])
    print(f"  Drill files exported to {out_dir}/")


def export_position() -> None:
    """Export pick-and-place CSV."""
    out = FAB / "pick-and-place.csv"
    FAB.mkdir(parents=True, exist_ok=True)
    _run([
        "kicad-cli", "pcb", "export", "pos",
        "--output", str(out),
        "--format", "csv",
        "--units", "mm",
        str(PCB),
    ])
    print(f"  Pick-and-place: {out}")


def main() -> None:
    if not PCB.exists() or PCB.stat().st_size < 200:
        sys.stderr.write(
            f"PCB at {PCB} is empty or missing. Run the pipeline first.\n"
        )
        sys.exit(1)
    
    print("=" * 60)
    print("DieselFire — Fabrication Export")
    print("=" * 60)
    
    run_drc()
    export_gerbers()
    export_drill()
    export_position()
    
    print(f"\nFab artifacts ready in {FAB}/")
    print("To assemble JLCPCB upload zip:")
    print(f"  cd {FAB} && zip DieselFire-v1-gerbers.zip gerbers/*.gbr drill/*.drl")


if __name__ == "__main__":
    main()
