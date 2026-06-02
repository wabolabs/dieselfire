# DieselFire S3 - PCB Layout Status

## Current State

The PCB is generated end-to-end by the automation pipeline
[`kicad/pipeline.py`](../../kicad/pipeline.py) (place → Freerouting autoroute →
GND pour → DRC → fab export). Run it with:

```bash
kicad/run_pipeline.sh --full      # or: python3 kicad/pipeline.py
```

### Completed (automated)

- **Board outline**: 100 × 100 mm, 1.6 mm, 2-layer, M3 mounting holes at the
  four corners (5,5)/(95,5)/(5,95)/(95,95).
- **Placement**: all 44 footprints placed from KiCad's built-in libraries.
- **Routing**: ~348 trace segments + routing vias via Freerouting.
- **Ground**: GND zones on both layers + ~371 stitching vias (grid + per-pad).
- **Display keepout**: the lid-mounted ILI9341 panel outline (50 × 69.2 mm) and
  active area (43.2 × 57.6 mm) are drawn on `Dwgs.User` with a label, so no tall
  part lands under the display. Driven by the `DISP_*` constants.
- **Side buttons**: SW1/SW2 are right-angle (side-actuated) tactile switches on
  the front edge; the case provides plunger holes.
- **Fab output**: gerbers, Excellon drill, and pick-and-place CSV in
  `kicad/fabrication/`.

### One manual step: fill the GND zones

The headless `pcbnew` used by CI/sandbox runs may lack a program context, in
which case `ZONE_FILLER` is unavailable and the pipeline saves the GND zones
**defined but unfilled** (it auto-detects this via a runtime probe). When that
happens, kicad-cli DRC reports the GND pads/vias as unconnected.

**To finish:** open `kicad/pcb/Afterburner-Modern.kicad_pcb` in KiCad and press
**B** (Edit ▸ Fill All Zones). This pours the planes and clears all GND
unconnected/dangling-via items, leaving only the two cosmetic silk-overlap
warnings on U8. In a full KiCad install the pipeline fills the zones
automatically and the saved board is already clean.

## Display & button mechanics

- The 2.8" ILI9341 panel mounts to the **inside of the case lid**, facing the
  user; the PCB sits behind it in the base. The FPC tail folds down to U11.
- Buttons are pressed from the **side of the case** through plunger holes, so
  the front face stays clear for the touchscreen.

See [`kicad/case/README.md`](../../kicad/case/README.md) for the enclosure and
[DieselFire-S3-Design.md](../design/DieselFire-S3-Design.md) for the schematic.

## Next steps

1. Open the PCB in KiCad and **Fill All Zones** (if generated headless).
2. Review the two silk-overlap warnings on U8 (cosmetic).
3. Regenerate gerbers (`kicad-cli` or the pipeline) and order from JLCPCB/PCBWay.
