#!/usr/bin/env bash
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
cd "$HERE"

PIPELINE="$HERE/pipeline.py"
APPLY="$HERE/pcb_placement/apply.py"
CRITICAL="$HERE/pcb_routing/critical.py"
FAB="$HERE/fabrication"
PCB="$HERE/pcb/Afterburner-Modern.kicad_pcb"

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTION]

Run the Afterburner-Modern PCB automation pipeline.

Options:
  --full          Full pipeline: place → autoroute → GND pour → DRC → export (default)
  --place-only    Place components only (no autorouting), then export
  --pre-route     Place + critical pre-routes + export (no autorouter)
  --clean         Remove all generated files (PCB, freerouting, fabrication)
  -h, --help      Show this help
EOF
    exit 0
}

clean() {
    echo "Cleaning generated files..."
    rm -rf "$HERE/freerouting"
    rm -rf "$FAB"
    rm -f "$PCB"
    echo "Done."
}

mode="full"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --full) mode="full"; shift ;;
        --place-only) mode="place"; shift ;;
        --pre-route) mode="preroute"; shift ;;
        --clean) clean; exit 0 ;;
        -h|--help) usage ;;
        *) echo "Unknown option: $1"; usage ;;
    esac
done

case "$mode" in
    full)
        echo "=== Full pipeline ==="
        python3 "$PIPELINE"
        ;;
    place)
        echo "=== Place-only pipeline ==="
        python3 -c "
import sys, pcbnew
from pathlib import Path
sys.path.insert(0, '$HERE')

from pipeline import (
    create_board_outline, create_nets, place_footprints,
    save_board, run_drc, export_fab, PCB_PATH
)

board = pcbnew.BOARD()
create_board_outline(board)
net_map = create_nets(board)
place_footprints(board, net_map)
save_board(board, PCB_PATH)
run_drc()
export_fab()
print('Place-only pipeline complete.')
"
        ;;
    preroute)
        echo "=== Pre-route pipeline ==="
        python3 "$PIPELINE" &
        PID=$!
        wait $PID
        echo "Running critical pre-routes..."
        python3 "$CRITICAL"
        echo "Applying placement plan..."
        python3 "$APPLY"
        echo "Pre-route pipeline complete."
        ;;
esac
