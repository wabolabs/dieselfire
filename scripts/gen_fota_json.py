#!/usr/bin/env python3
"""Generate fota.json manifest for DieselFire OTA updates.

Usage:
  python gen_fota_json.py \
    --type dieselfire-fota-http \
    --version 3.3.0 \
    --host dieselfire.wabo.cc \
    --port 80 \
    --bin "/fota/dieselfire-3.3.0.bin" \
    [--output fota.json]
"""

import argparse
import json
import re


def parse_version(version_str: str) -> int:
    """Convert semver string to the integer format esp32FOTA expects.
    
    The firmware uses: int((revision * 0.1 + sub * 0.001) * 1000)
    So V3.3 -> revision=33, sub=0 -> 33*100 + 0 = 3300
    V3.3.1 -> revision=33, sub=0, minor=1 -> beta -> 3300
    
    The tag format is v3.3.0 or v3.3.0-beta1
    """
    m = re.match(r'(\d+)\.(\d+)(?:\.(\d+))?(?:-beta(\d+))?', version_str)
    if not m:
        raise ValueError(f"Cannot parse version: {version_str}")
    
    major = int(m.group(1))
    minor = int(m.group(2))
    patch = int(m.group(3)) if m.group(3) else 0
    beta = int(m.group(4)) if m.group(4) else 0
    
    # DieselFire encodes version as revision*100 + sub
    # where revision = major*10 + minor, sub = patch
    revision = major * 10 + minor
    sub = patch
    
    version_int = revision * 100 + sub
    return version_int


def main():
    parser = argparse.ArgumentParser(description='Generate fota.json manifest')
    parser.add_argument('--type', required=True, help='Firmware type string')
    parser.add_argument('--version', required=True, help='Semver version (e.g. 3.3.0)')
    parser.add_argument('--host', required=True, help='Download server hostname')
    parser.add_argument('--port', type=int, default=80, help='Download server port')
    parser.add_argument('--bin', required=True, help='Path to firmware binary on server')
    parser.add_argument('--output', default='fota.json', help='Output file path')
    
    args = parser.parse_args()
    
    manifest = {
        "type": args.type,
        "version": parse_version(args.version),
        "host": args.host,
        "port": args.port,
        "bin": args.bin
    }
    
    with open(args.output, 'w') as f:
        json.dump(manifest, f, indent=2)
    
    print(f"Generated {args.output}:")
    print(json.dumps(manifest, indent=2))


if __name__ == '__main__':
    main()
