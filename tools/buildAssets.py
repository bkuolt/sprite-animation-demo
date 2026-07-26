#!/usr/bin/env python3
import os
import glob
import subprocess
import sys

def build_shaders():
    print("Building Shaders...")
    shader_dir = "assets/shaders"
    shaders = [
        f"{shader_dir}/main.vs",
        f"{shader_dir}/main.fs",
        f"{shader_dir}/text.vs",
        f"{shader_dir}/text.fs"
    ]
    
    for shader in shaders:
        if shader.endswith(".vs"):
            out = shader.replace(".vs", ".vert.spv")
            stage = "vert"
        else:
            out = shader.replace(".fs", ".frag.spv")
            stage = "frag"
            
        print(f"Compiling {shader} -> {out}")
        res = subprocess.run(["glslangValidator", "-G", "-S", stage, shader, "-o", out])
        if res.returncode != 0:
            print(f"Error compiling {shader}", file=sys.stderr)
            sys.exit(1)

def build_textures():
    print("Building Textures...")
    png_dir = "assets/textures/png"
    ktx_dir = "assets/textures/ktx"
    pngs = glob.glob(f"{png_dir}/*.png")
    
    for png in pngs:
        basename = os.path.basename(png)
        out = os.path.join(ktx_dir, basename.replace(".png", ".ktx2"))
        
        print(f"Converting {png} -> {out}")
        # toktx --t2 --uastc 2 --zcmp 9 --genmipmap out.ktx2 in.png
        res = subprocess.run([
            "toktx", "--t2", "--uastc", "2", "--zcmp", "9", "--genmipmap", out, png
        ])
        if res.returncode != 0:
            print(f"Error converting {png}", file=sys.stderr)
            sys.exit(1)

if __name__ == "__main__":
    build_shaders()
    # build_textures() # Requires toktx
    print("Asset build complete.")
