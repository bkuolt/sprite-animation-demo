#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

import os
import re
import subprocess
import sys
from pathlib import Path
from collections import defaultdict

def natural_sort_key(s):
    """Sort strings with embedded numbers naturally (e.g. 2 before 10)"""
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(r'(\d+)', str(s))]

def process_character_animations(png_dir: Path, ktx_dir: Path):
    """
    Finds all PNG frames for characters in png_dir, groups them by animation,
    sorts them numerically, and combines them into 2D Array KTX2 files.
    """
    if not png_dir.exists():
        print(f"Error: Directory '{png_dir}' does not exist.")
        sys.exit(1)
    
    # Ensure toktx is available and LD_LIBRARY_PATH is set (handled in install.sh, but we can try)
    try:
        subprocess.run(["toktx", "--version"], capture_output=True, check=True)
    except Exception:
        print("Error: 'toktx' command not found or fails to run.")
        print("Please run ./install.sh to install KTX-Software.")
        sys.exit(1)

    characters = [d for d in png_dir.iterdir() if d.is_dir()]
    
    for char_dir in characters:
        char_name = char_dir.name
        
        # Group PNG files by animation prefix (e.g., "Dead (1).png" -> "Dead")
        # Assuming format: "AnimationName (Number).png" or "AnimationName_Number.png"
        animations = defaultdict(list)
        for img in char_dir.glob("*.png"):
            # Strip number and extension
            match = re.match(r'([A-Za-z_]+)', img.name)
            if match:
                anim_name = match.group(1).strip()
                animations[anim_name].append(img)
                
        # For each animation, sort naturally and generate KTX2
        for anim_name, frames in animations.items():
            # Natural sort frames
            frames.sort(key=lambda p: natural_sort_key(p.name))
            
            out_dir = ktx_dir / char_name
            out_dir.mkdir(parents=True, exist_ok=True)
            
            # Use lowercase for standardizing
            out_file = out_dir / f"{anim_name.lower()}.ktx2"
            
            cmd = [
                "toktx",
                "--t2",
                "--zcmp", "20",
                "--genmipmap",
                str(out_file)
            ] + [str(f) for f in frames]
            
            print(f"[{char_name}] Generating {out_file.name} from {len(frames)} frames...")
            try:
                subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL)
            except subprocess.CalledProcessError as e:
                print(f"Failed to generate {out_file}: {e}")
                sys.exit(1)

def main():
    base_dir = Path(__file__).resolve().parent.parent
    png_dir = base_dir / "assets" / "textures" / "png"
    ktx_dir = base_dir / "assets" / "textures" / "ktx"
    
    print("Starting PNG to KTX2 pipeline...")
    process_character_animations(png_dir, ktx_dir)
    print("Pipeline complete!")

if __name__ == "__main__":
    main()
