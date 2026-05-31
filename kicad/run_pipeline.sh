#!/usr/bin/env bash
# DieselFire S3 — Full PCB Automation Pipeline
#
# Usage: ./kicad/run_pipeline.sh
#
# This script runs the complete PCB generation pipeline:
#   1. Create board outline, zones, vias via pcbnew API
#   2. Load KiCad 9.0 built-in footprints and place them
#   3. Assign nets to pads
#   4. Route critical traces between pad centers
#   5. Fill GND zones with stitching vias
#   6. Run DRC
#   7. Export gerbers/drill/CPL
#
# Requires: KiCad 9.0+, Python 3.10+
#
# Output: kicad/fabrication/ (gerbers, drill, pick-and-place)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "============================================================"
echo "DieselFire S3 — PCB Automation Pipeline"
echo "============================================================"
echo ""

# Check prerequisites
echo "Checking prerequisites..."
for cmd in python3 kicad-cli; do
    if ! command -v "$cmd" &>/dev/null; then
        echo "ERROR: $cmd not found."
        exit 1
    fi
done
echo "  ✓ All prerequisites available"
echo ""

# Run the pipeline
python3 "$SCRIPT_DIR/pipeline.py"
