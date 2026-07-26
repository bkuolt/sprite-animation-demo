#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# Copyright (c) 2024-2026 Bastian. All rights reserved.

import argparse
import subprocess
import sys
from pathlib import Path

def convert_to_ktx2(input_path: Path, output_path: Path):
    """
    Converts a PNG image into a KTX2 texture array using Basis Universal
    compression and Zstandard.
    """
    if not input_path.exists():
        print(f"Error: Input file '{input_path}' does not exist.")
        sys.exit(1)
    
    # Ensure output directory exists
    output_path.parent.mkdir(parents=True, exist_ok=True)

    # Command for toktx:
    # --t2: Create a KTX2 file
    # --bcmp: Use Basis Universal supercompression
    # --zcmp 20: Use Zstandard supercompression level 20
    # --genmipmap: Generate mipmaps
    cmd = [
        "toktx",
        "--t2",
        "--bcmp",
        "--zcmp", "20",
        "--genmipmap",
        str(output_path),
        str(input_path)
    ]

    print(f"Converting '{input_path}' to '{output_path}'...")
    try:
        subprocess.run(cmd, check=True)
        print(f"Successfully created '{output_path}'")
    except subprocess.CalledProcessError as e:
        print(f"Error during conversion: {e}")
        sys.exit(1)
    except FileNotFoundError:
        print("Error: 'toktx' command not found. Please install KTX-Software tools.")
        sys.exit(1)

def main():
    parser = argparse.ArgumentParser(description="Convert PNG images to KTX2 textures using Basis Universal and Zstd.")
    parser.add_argument("input", type=Path, help="Input PNG file")
    parser.add_argument("-o", "--output", type=Path, help="Output KTX2 file (optional, defaults to same name with .ktx2 extension)")
    
    args = parser.parse_args()
    
    input_path = args.input
    output_path = args.output
    
    if not output_path:
        output_path = input_path.with_suffix(".ktx2")
        
    convert_to_ktx2(input_path, output_path)

if __name__ == "__main__":
    main()
