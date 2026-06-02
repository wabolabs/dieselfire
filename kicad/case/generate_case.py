#!/usr/bin/env python3
"""
DieselFire S3 — parametric 3D-printable enclosure generator.

Generates a two-part screw enclosure (base tray + lid) sized to the
100 x 100 mm KiCad PCB (Afterburner-Modern.kicad_pcb), with:
  - corner standoffs aligned to the four M3 mounting holes at (5,5)/(95,5)/(5,95)/(95,95)
  - cable-exit cutouts on the walls aligned to the real connector positions
  - a lid-mounted ILI9341 2.8" display: active-area window + recessed glass
    pocket with bezel ledge, retaining rib and FPC exit notch (DISP_* params,
    mirrored from the board keepout in ../pipeline.py)
  - side-button plunger holes for the right-angle SW1/SW2 tactiles
  - ventilation slots on base and lid
  - M3 screw bosses joining lid to base

Board coordinate system (KiCad, mm, Y increases "downward"):
    origin (0,0) = top-left corner of board, board spans 0..100 in X and Y.
The case uses the same X/Y so connector positions map directly. Z is up,
with the PCB sitting on top of the standoffs inside the base tray.

Outputs (written next to this script in kicad/case/):
    dieselfire_case_base.stl / .step
    dieselfire_case_lid.stl  / .step
    dieselfire_case_assembled.step   (both parts, for preview)

Run:  python3 generate_case.py
"""

import os
import cadquery as cq
from cadquery import exporters

# --------------------------------------------------------------------------
# Parameters (mm)
# --------------------------------------------------------------------------
BOARD_X = 100.0
BOARD_Y = 100.0
BOARD_T = 1.6           # PCB thickness
PCB_CLEAR = 0.6         # gap between PCB edge and inner wall (per side)

WALL = 2.4              # outer wall thickness
FLOOR = 2.0             # base floor thickness
LID_TOP = 2.0           # lid top thickness

STANDOFF_H = 10.0       # PCB sits this high above the floor (room for B.Cu connectors)
PCB_TO_LID = 16.0       # clear height above PCB top (display hangs from lid into here)
LID_LIP = 4.0           # how far the lid skirt overlaps the base wall

STANDOFF_OD = 7.0       # outer dia of PCB standoff
STANDOFF_ID = 2.6       # pilot hole for M3 self-tapping screw into standoff
SCREW_BOSS_OD = 8.0     # lid-to-base screw boss outer dia
SCREW_CLEAR = 3.4       # M3 clearance hole through lid bosses
SCREW_PILOT = 2.6       # M3 self-tap pilot in base bosses

MOUNT_HOLES = [(5, 5), (95, 5), (5, 95), (95, 95)]

CORNER_R = 4.0          # outer corner radius

# Connector cutouts: (name, edge, center_along_edge, width, z_bottom, z_height)
#  edge: 'front' (Y~0), 'back' (Y~100), 'left' (X~0), 'right' (X~100)
#  center_along_edge: position along that wall in board mm (X for front/back, Y for left/right)
#  z measured from PCB top surface (0 = PCB top). Cutout spans the wall thickness.
# Connector board extents were measured from the PCB courtyards:
#   J2 term-block  X[9.5,18.5]   front
#   J4 JST3        X[22.1,33.0]  front
#   J5 JST4        X[39.0,52.5]  front
#   J1 USB-C       X[80.7,90.2]  front
#   J3 JST3        Y[63.1,69.9]  left
#   H1 hdr 2x10    Y[22.4,56.5]  right
#   H2 hdr 2x10    Y[64.4,98.5]  right
#   U11 FH12 FPC   X[42.5,57.5]  back
CONNECTOR_CUTOUTS = [
    # front wall (Y ~ 0) — power + sensor harnesses
    ("J2_12V",  "front", (9.5 + 18.5) / 2,  12.0, -1.0, 12.0),
    ("J4_DS18",  "front", (22.1 + 33.0) / 2, 12.0, -1.0, 10.0),
    ("J5_MQ7",   "front", (39.0 + 52.5) / 2, 15.0, -1.0, 10.0),
    ("J1_USBC",  "front", (80.7 + 90.2) / 2, 12.0, -1.0,  6.0),
    # left wall (X ~ 0) — blue wire to heater
    ("J3_BLUE",  "left",  (63.1 + 69.9) / 2, 12.0, -1.0, 10.0),
    # right wall (X ~ 100) — expansion headers (ribbon access)
    ("H1_EXP",   "right", (22.4 + 56.5) / 2, 38.0, -1.0, 12.0),
    ("H2_EXP",   "right", (64.4 + 98.5) / 2, 38.0, -1.0, 12.0),
]

# ---- ILI9341 2.8" display, mounted to the lid (faces user) ----
# Values mirror the board keepout in kicad/pipeline.py.  The panel drops into a
# pocket on the inside of the lid; its glass seats against a bezel ledge and the
# active area shows through the window.  The FPC tail exits the +Y module edge
# (board "back") and folds down into the cavity to U11.
DISP_CX = 50.0          # display centre on the board (X) -> maps via b2w()
DISP_CY = 50.0          # display centre on the board (Y)
DISP_ACT_W = 43.2       # active-area window width  (portrait)
DISP_ACT_H = 57.6       # active-area window height
DISP_MOD_W = 50.0       # glass/module pocket width
DISP_MOD_H = 69.2       # glass/module pocket height
DISP_POCKET_FIT = 0.4   # per-side clearance around the glass in the pocket
DISP_GLASS_T = 2.2      # glass + bonded touch thickness -> pocket depth from inner face
DISP_RIB_H = 3.5        # retaining-rib height below the bezel ledge (into cavity)
DISP_RIB_W = 1.6        # retaining-rib wall thickness
DISP_FPC_W = 14.0       # width of the FPC exit notch in the rib (on +Y edge)

# Side buttons: right-angle tactiles on the front edge, actuators facing -Y.
# Plunger access holes in the front wall, centred on each switch body.
BUTTON_HOLE_D = 7.0     # access hole diameter
BUTTON_Z = 3.5          # actuator centreline height above PCB top
SIDE_BUTTONS = [
    ("SW1", 62.3),      # board-X of the plunger
    ("SW2", 73.3),
]

OUT_DIR = os.path.dirname(os.path.abspath(__file__))

# --------------------------------------------------------------------------
# Derived geometry
# --------------------------------------------------------------------------
inner_x = BOARD_X + 2 * PCB_CLEAR
inner_y = BOARD_Y + 2 * PCB_CLEAR
outer_x = inner_x + 2 * WALL
outer_y = inner_y + 2 * WALL

# Base inner cavity height (floor top -> top rim of base wall)
base_wall_h = STANDOFF_H + BOARD_T + PCB_TO_LID
# inner-corner offset so board (0..100) maps into the cavity
# board local (0,0) -> world (WALL+PCB_CLEAR, WALL+PCB_CLEAR)
OX = WALL + PCB_CLEAR
OY = WALL + PCB_CLEAR
PCB_TOP_Z = FLOOR + STANDOFF_H + BOARD_T   # world Z of PCB top surface


def b2w(bx, by):
    """board (mm) -> world XY (mm)."""
    return (OX + bx, OY + by)


def rrect(w, h, r):
    """centered rounded rectangle wire on XY plane."""
    return cq.Workplane("XY").rect(w, h).val()  # placeholder, not used


# --------------------------------------------------------------------------
# BASE
# --------------------------------------------------------------------------
def make_base():
    # Outer shell
    base = (
        cq.Workplane("XY")
        .box(outer_x, outer_y, FLOOR + base_wall_h, centered=(False, False, False))
        .edges("|Z").fillet(CORNER_R)
    )
    # Hollow the cavity (leave floor + walls)
    cavity = (
        cq.Workplane("XY")
        .workplane(offset=FLOOR)
        .moveTo(WALL, WALL)
        .box(inner_x, inner_y, base_wall_h + 1, centered=(False, False, False))
        .edges("|Z").fillet(max(CORNER_R - WALL, 0.5))
    )
    base = base.cut(cavity)

    # PCB standoffs at mount holes
    for (bx, by) in MOUNT_HOLES:
        wx, wy = b2w(bx, by)
        post = (
            cq.Workplane("XY").workplane(offset=FLOOR)
            .moveTo(wx, wy).circle(STANDOFF_OD / 2)
            .extrude(STANDOFF_H)
        )
        pilot = (
            cq.Workplane("XY").workplane(offset=FLOOR + STANDOFF_H)
            .moveTo(wx, wy).circle(STANDOFF_ID / 2)
            .extrude(-(STANDOFF_H - 0.6))
        )
        base = base.union(post).cut(pilot)

    # Lid screw bosses in the four corners of the wall (between standoffs & wall)
    boss_positions = corner_boss_positions()
    for (wx, wy) in boss_positions:
        boss = (
            cq.Workplane("XY").workplane(offset=FLOOR)
            .moveTo(wx, wy).circle(SCREW_BOSS_OD / 2)
            .extrude(base_wall_h)
        )
        pilot = (
            cq.Workplane("XY").workplane(offset=FLOOR + base_wall_h)
            .moveTo(wx, wy).circle(SCREW_PILOT / 2)
            .extrude(-12.0)
        )
        base = base.union(boss).cut(pilot)

    # Connector cutouts in the walls
    base = cut_connectors(base, top_z_ref=PCB_TOP_Z, wall_top=FLOOR + base_wall_h)

    # Side-button plunger holes (front wall)
    base = cut_side_buttons(base)

    # Ventilation slots in the floor (under the ESP32 / regulators)
    base = vent_slots_floor(base)

    return base


def cut_side_buttons(solid):
    """Drill plunger access holes through the front wall for the side buttons."""
    z = PCB_TOP_Z + BUTTON_Z
    for _name, bx in SIDE_BUTTONS:
        wx, _ = b2w(bx, 0)
        cutter = (
            cq.Workplane("XY").circle(BUTTON_HOLE_D / 2)
            .extrude(WALL + 4)
            .rotate((0, 0, 0), (1, 0, 0), -90)   # reorient cylinder along +Y
            .translate((wx, -2, z))
        )
        solid = solid.cut(cutter)
    return solid


def corner_boss_positions():
    inset = WALL + SCREW_BOSS_OD / 2 + 0.4
    return [
        (inset, inset),
        (outer_x - inset, inset),
        (inset, outer_y - inset),
        (outer_x - inset, outer_y - inset),
    ]


def cut_connectors(solid, top_z_ref, wall_top):
    for (name, edge, pos, width, z0, zh) in CONNECTOR_CUTOUTS:
        z_bottom = top_z_ref + z0
        z_top = z_bottom + zh
        z_top = min(z_top, wall_top + 1)
        h = z_top - z_bottom
        cx_b = cy_b = 0.0
        if edge in ("front", "back"):
            wx, _ = b2w(pos, 0)
            cy = WALL / 2 if edge == "front" else outer_y - WALL / 2
            cutter = (
                cq.Workplane("XY").workplane(offset=z_bottom)
                .moveTo(wx, cy).rect(width, WALL * 3)
                .extrude(h)
            )
        else:  # left / right
            _, wy = b2w(0, pos)
            cx = WALL / 2 if edge == "left" else outer_x - WALL / 2
            cutter = (
                cq.Workplane("XY").workplane(offset=z_bottom)
                .moveTo(cx, wy).rect(WALL * 3, width)
                .extrude(h)
            )
        solid = solid.cut(cutter)
    return solid


def vent_slots_floor(solid):
    # a 4 x 3 grid of slots in the centre of the floor
    slot_w, slot_l, gap = 2.0, 18.0, 4.0
    nx, ny = 5, 3
    total_w = nx * slot_w + (nx - 1) * gap
    cx0 = outer_x / 2 - total_w / 2 + slot_w / 2
    cy = outer_y / 2
    for i in range(nx):
        x = cx0 + i * (slot_w + gap)
        slot = (
            cq.Workplane("XY")
            .moveTo(x, cy).rect(slot_w, slot_l)
            .extrude(FLOOR + 0.2)
        )
        solid = solid.cut(slot)
    return solid


# --------------------------------------------------------------------------
# LID
# --------------------------------------------------------------------------
def make_lid():
    lid_h = LID_TOP + LID_LIP
    # Top plate (full outer footprint)
    lid = (
        cq.Workplane("XY")
        .box(outer_x, outer_y, LID_TOP, centered=(False, False, False))
        .edges("|Z").fillet(CORNER_R)
    )
    # Skirt that drops down into the base wall (sits just inside outer wall)
    skirt_out_x, skirt_out_y = outer_x, outer_y
    skirt = (
        cq.Workplane("XY").workplane(offset=LID_TOP)
        .box(outer_x, outer_y, LID_LIP, centered=(False, False, False))
        .edges("|Z").fillet(CORNER_R)
    )
    inner_skirt = (
        cq.Workplane("XY").workplane(offset=LID_TOP)
        .moveTo(WALL, WALL)
        .box(inner_x, inner_y, LID_LIP + 1, centered=(False, False, False))
        .edges("|Z").fillet(max(CORNER_R - WALL, 0.5))
    )
    skirt = skirt.cut(inner_skirt)
    lid = lid.union(skirt)

    # Screw bosses on lid (clearance holes, countersunk on top)
    for (wx, wy) in corner_boss_positions():
        boss = (
            cq.Workplane("XY").workplane(offset=LID_TOP)
            .moveTo(wx, wy).circle(SCREW_BOSS_OD / 2)
            .extrude(LID_LIP)
        )
        lid = lid.union(boss)
        hole = (
            cq.Workplane("XY")
            .moveTo(wx, wy).circle(SCREW_CLEAR / 2)
            .extrude(LID_TOP + LID_LIP + 1)
        )
        lid = lid.cut(hole)
        # counterbore for screw head on the top face
        cbore = (
            cq.Workplane("XY")
            .moveTo(wx, wy).circle(6.0 / 2)
            .extrude(1.6)
        )
        lid = lid.cut(cbore)

    # ---- ILI9341 display mount ----
    lid = add_display_mount(lid)

    # Ventilation slots on the lid, in the back margin clear of the window
    slot_w, slot_l, gap = 2.0, 26.0, 4.0
    nx = 6
    total_w = nx * slot_w + (nx - 1) * gap
    cx0 = outer_x / 2 - total_w / 2 + slot_w / 2
    cy = OY + 92.0  # back margin, beyond the display window
    for i in range(nx):
        x = cx0 + i * (slot_w + gap)
        slot = (
            cq.Workplane("XY")
            .moveTo(x, cy).rect(slot_w, slot_l)
            .extrude(LID_TOP + 0.2)
        )
        lid = lid.cut(slot)

    return lid


def add_display_mount(lid):
    """Cut the active-area window and build the glass pocket + FPC notch.

    The panel drops into the pocket from the cavity side, its glass seating
    against the bezel ledge; the active area shows through the window. A
    retaining rib walls the module outline; a notch on the +Y edge lets the
    FPC tail fold down to U11.
    """
    cx, cy = b2w(DISP_CX, DISP_CY)

    # See-through window (active area) through the top plate
    win = (
        cq.Workplane("XY")
        .moveTo(cx, cy).rect(DISP_ACT_W, DISP_ACT_H)
        .extrude(LID_TOP + 0.2)
    )
    lid = lid.cut(win)

    # Glass pocket: recess the module outline from the inner face so the glass
    # sits flush against the bezel ledge (keeps the front face proud of glass).
    pocket_w = DISP_MOD_W + 2 * DISP_POCKET_FIT
    pocket_h = DISP_MOD_H + 2 * DISP_POCKET_FIT
    pocket = (
        cq.Workplane("XY").workplane(offset=LID_TOP - DISP_GLASS_T)
        .moveTo(cx, cy).rect(pocket_w, pocket_h)
        .extrude(DISP_GLASS_T + 0.1)
    )
    lid = lid.cut(pocket)

    # Retaining rib around the pocket (projects into the cavity)
    rib_outer = (
        cq.Workplane("XY").workplane(offset=LID_TOP)
        .moveTo(cx, cy).rect(pocket_w + 2 * DISP_RIB_W, pocket_h + 2 * DISP_RIB_W)
        .extrude(DISP_RIB_H)
    )
    rib_inner = (
        cq.Workplane("XY").workplane(offset=LID_TOP - 0.1)
        .moveTo(cx, cy).rect(pocket_w, pocket_h)
        .extrude(DISP_RIB_H + 0.2)
    )
    rib = rib_outer.cut(rib_inner)

    # FPC exit notch on the +Y edge of the rib (board "back", toward U11)
    notch = (
        cq.Workplane("XY").workplane(offset=LID_TOP - 0.1)
        .moveTo(cx, cy + pocket_h / 2 + DISP_RIB_W)
        .rect(DISP_FPC_W, DISP_RIB_W * 3)
        .extrude(DISP_RIB_H + 0.2)
    )
    rib = rib.cut(notch)
    lid = lid.union(rib)

    return lid


# --------------------------------------------------------------------------
# Export
# --------------------------------------------------------------------------
def main():
    print("Generating base ...")
    base = make_base()
    print("Generating lid ...")
    lid = make_lid()

    base_path = os.path.join(OUT_DIR, "dieselfire_case_base")
    lid_path = os.path.join(OUT_DIR, "dieselfire_case_lid")

    exporters.export(base, base_path + ".stl", tolerance=0.05, angularTolerance=0.2)
    exporters.export(base, base_path + ".step")
    exporters.export(lid, lid_path + ".stl", tolerance=0.05, angularTolerance=0.2)
    exporters.export(lid, lid_path + ".step")

    # Assembled preview: lid flipped onto base
    lid_top_z = FLOOR + base_wall_h
    lid_flipped = (
        lid.rotate((0, 0, 0), (1, 0, 0), 180)
        .translate((0, outer_y, lid_top_z + LID_TOP + LID_LIP))
    )
    assembly = base.union(lid_flipped)
    exporters.export(assembly, os.path.join(OUT_DIR, "dieselfire_case_assembled.step"))

    for p in (base_path + ".stl", lid_path + ".stl"):
        print(f"  {os.path.basename(p)}: {os.path.getsize(p)/1024:.0f} kB")
    print("Done. Outer envelope: "
          f"{outer_x:.1f} x {outer_y:.1f} x {FLOOR + base_wall_h + LID_TOP:.1f} mm")


if __name__ == "__main__":
    main()
