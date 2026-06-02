#!/usr/bin/env python3
"""Apply the placement plan to Afterburner-Modern.kicad_pcb.

Reads the .kicad_pcb file, finds each footprint's reference, looks it up
in plan.OVERRIDES or plan.SHEET_ZONES, and rewrites the (at X Y rot) and
(layer ...) fields. Idempotent — safe to re-run.

Run from project root:
    python3 kicad/pcb_placement/apply.py
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
PROJECT_ROOT = HERE.parents[0]  # kicad/
PCB_PATH = PROJECT_ROOT / "pcb" / "Afterburner-Modern.kicad_pcb"

sys.path.insert(0, str(HERE))
from plan import OVERRIDES, SHEET_ZONES, DEFAULT_PARK, get_placement


def tokenize(text: str) -> list[str]:
    """Tokenize KiCad S-expression text into atoms and parentheses."""
    tokens = []
    i = 0
    while i < len(text):
        c = text[i]
        if c.isspace():
            i += 1
        elif c == '(':
            tokens.append('(')
            i += 1
        elif c == ')':
            tokens.append(')')
            i += 1
        elif c == '"':
            j = text.index('"', i + 1)
            tokens.append(text[i:j+1])
            i = j + 1
        else:
            j = i
            while j < len(text) and text[j] not in '() \t\n\r':
                j += 1
            tokens.append(text[i:j])
            i = j
    return tokens


def parse_tree(tokens: list[str], pos: int = 0) -> tuple:
    """Parse tokens into nested tuples. Returns (tree, next_pos)."""
    if pos >= len(tokens):
        return (None, pos)
    if tokens[pos] == '(':
        children = []
        pos += 1
        while pos < len(tokens) and tokens[pos] != ')':
            child, pos = parse_tree(tokens, pos)
            children.append(child)
        pos += 1  # skip ')'
        return (children, pos)
    elif tokens[pos] == ')':
        return (None, pos + 1)
    else:
        return (tokens[pos], pos + 1)


def find_footprints(text: str) -> list[dict]:
    """Find all footprint blocks with their positions in the original text.
    Returns list of dicts with key, ref, at_x, at_y, at_rot, layer, start, end."""
    footprints = []
    # Find all (footprint "..." ...) blocks by scanning for the pattern
    # and tracking parenthesis depth.
    i = 0
    while i < len(text):
        # Look for next (footprint
        idx = text.find('(footprint "', i)
        if idx == -1:
            break
        
        # Parse the key
        key_start = idx + len('(footprint "')
        key_end = text.index('"', key_start)
        key = text[key_start:key_end]
        
        # Find the end of this footprint block by tracking parens
        depth = 0
        j = idx
        while j < len(text):
            if text[j] == '(':
                depth += 1
            elif text[j] == ')':
                depth -= 1
                if depth == 0:
                    break
            j += 1
        
        block = text[idx:j+1]
        
        # Extract reference from first (property "Reference" ...)
        ref_match = re.search(
            r'\(property\s+"Reference"\s+"([^"]+)"',
            block,
        )
        ref = ref_match.group(1) if ref_match else None
        
        # Extract position from top-level (at X Y [rot])
        # The top-level (at) appears before any (property)
        prop_idx = block.find('(property', 1)
        if prop_idx > 0:
            at_match = re.search(r'\(at\s+(-?[\d.]+)\s+(-?[\d.]+)(?:\s+(\d+))?\)', block[:prop_idx])
        else:
            at_match = re.search(r'\(at\s+(-?[\d.]+)\s+(-?[\d.]+)(?:\s+(\d+))?\)', block)
        
        at_x = float(at_match.group(1)) if at_match else 0
        at_y = float(at_match.group(2)) if at_match else 0
        at_rot = float(at_match.group(3)) if at_match and at_match.group(3) else 0.0
        
        # Extract layer from top-level (layer "...")
        layer_match = re.search(r'\(layer\s+"([^"]+)"', block[:prop_idx]) if prop_idx > 0 else None
        if not layer_match:
            layer_match = re.search(r'\(layer\s+"([^"]+)"', block)
        layer = layer_match.group(1) if layer_match else "F.Cu"
        
        footprints.append({
            "key": key,
            "ref": ref,
            "at_x": at_x,
            "at_y": at_y,
            "at_rot": at_rot,
            "layer": layer,
            "block": block,
            "start": idx,
            "end": j + 1,
        })
        
        i = j + 1
    
    return footprints


def determine_zone(ref: str, block: str) -> str | None:
    """Determine which functional zone a component belongs to."""
    ref_prefix = ""
    if ref:
        ref_prefix = re.match(r'^([A-Z]+)\d*[_\d]*', ref).group(1) if ref else ""
    
    zone_map = {
        "J2": "power_r",
        "J1": "ext_sensors",
        "U2": "power_r",
        "U3": "power_r",
        "U1": "mcu_c",
        "U11": "display",
        "U10": "display",
        "U4": "sensor_c",
        "U7": "sensor_c",
        "BAT1": "sensor_c",
        "U9": "io_c",
        "U8": "io_c",
        "J3": "heater",
        "J4": "ext_sensors",
        "J5": "ext_sensors",
        "H1": "expansion",
        "H2": "expansion",
        "SW1": "mcu_c",
        "SW2": "mcu_c",
        "D1": "mcu_c",
        "D2": "mcu_c",
        "R": "power_r",
        "C": "mcu_c",
    }
    
    if ref in zone_map:
        return zone_map[ref]
    if ref_prefix in zone_map:
        return zone_map[ref_prefix]
    
    fp_zone_map = {
        "TERM_BLOCK_2POS": "power_r",
        "USB-C-31-SR": "ext_sensors",
        "MP2451": "power_r",
        "AP2112": "power_r",
        "ESP32-S3-WROOM-1-N8R8": "mcu_c",
        "GT911": "display",
        "BME280": "sensor_c",
        "DS3231": "sensor_c",
        "74LCX125": "io_c",
        "BSS138": "io_c",
        "JST-XH-3": "heater",
        "JST-XH-4": "ext_sensors",
        "IDC-Header_2x10": "expansion",
        "SW_Push_1TS009": "mcu_c",
        "LED_0603": "mcu_c",
    }
    
    for key_prefix, zone in fp_zone_map.items():
        if key_prefix in block:
            return zone
    
    return None


def rewrite_footprint_block(block: str, x: float, y: float, rot: float, layer: str) -> str:
    """Rewrite the (at X Y [rot]) and (layer "F.Cu") in a footprint block.
    
    Uses regex on the raw block text (not line-splitting) to only modify the
    first (at ...) and (layer ...) that appear before any (property ...) entry.
    This avoids corrupting pad-level (at ...) or property-level (at ...) entries.
    """
    prop_idx = block.find('(property', 1)
    search_area = block[:prop_idx] if prop_idx > 0 else block
    
    at_re = r'\(at\s+(-?[\d.]+)\s+(-?[\d.]+)(?:\s+(-?[\d.]+))?\)'
    at_match = re.search(at_re, search_area)
    if at_match:
        new_at = f'(at {int(x)} {int(y)}'
        if abs(rot) > 0.001:
            new_at += f' {int(round(rot))}'
        new_at += ')'
        block = block[:at_match.start()] + new_at + block[at_match.end():]
    
    prop_idx = block.find('(property', 1)
    search_area = block[:prop_idx] if prop_idx > 0 else block
    layer_match = re.search(r'\(layer\s+"([^"]+)"\)', search_area)
    if layer_match:
        block = block[:layer_match.start()] + f'(layer "{layer}")' + block[layer_match.end():]
    
    return block


def apply_plan(pcb_path: Path = PCB_PATH) -> tuple[int, int, list[str]]:
    """Apply placement plan to the PCB file."""
    text = pcb_path.read_text()
    footprints = find_footprints(text)
    
    if not footprints:
        print(f"warning: no footprints found in {pcb_path}")
        return (0, 0, [])
    
    print(f"Found {len(footprints)} footprints to place")
    
    zone_index: dict[str, int] = {}
    placed = 0
    parked = 0
    missing = []
    
    replacements = []
    for fp in footprints:
        ref = fp["ref"]
        if ref is None:
            continue
        if ref.startswith("#"):
            continue
        
        zone = determine_zone(ref, fp["block"])
        idx = zone_index.get(zone or "", 0)
        zone_index[zone or ""] = idx + 1
        
        x, y, rot, layer = get_placement(ref, zone, idx)
        
        new_block = rewrite_footprint_block(fp["block"], x, y, rot, layer)
        replacements.append((fp["start"], fp["end"], new_block))
        
        if ref in OVERRIDES:
            placed += 1
        elif zone in SHEET_ZONES:
            placed += 1
        else:
            parked += 1
    
    # Apply replacements in reverse order to preserve positions
    for start, end, new_block in reversed(replacements):
        text = text[:start] + new_block + text[end:]
    
    pcb_path.write_text(text)
    
    for fp in footprints:
        if fp["ref"] and not fp["ref"].startswith("#"):
            zone = determine_zone(fp["ref"], fp["block"])
            if zone not in SHEET_ZONES and fp["ref"] not in OVERRIDES:
                missing.append(fp["ref"])
    
    return (placed, parked, missing)


if __name__ == "__main__":
    placed, parked, missing = apply_plan()
    print(f"\nplaced:  {placed:>4} components in planned positions")
    print(f"parked:  {parked:>4} components off-board (unrecognized zone)")
    if missing:
        print(f"missing: {len(missing)} refs not found: {missing[:5]}")
    print(f"\nPlacement applied to {PCB_PATH}")
