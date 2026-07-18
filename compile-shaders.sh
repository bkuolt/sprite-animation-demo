#!/bin/bash
#
# This script compiles the project's GLSL shaders into the SPIR-V format.
# Aborts immediately on error.
set -e

# --- Configuration ---
COMPILER="glslangValidator"
SRC_DIR="src"
OUT_DIR="assets"

VERTEX_SHADER="$SRC_DIR/main.vs"
FRAGMENT_SHADER="$SRC_DIR/main.fs"

# --- Logic ---

# 1. Check if the compiler is available
if ! command -v $COMPILER &> /dev/null
then
    echo "Error: '$COMPILER' not found."
    echo "Please install 'glslang-tools' (e.g., with 'sudo apt install glslang-tools')."
    exit 1
fi

# 2. Check if the source files exist
if [ ! -f "$VERTEX_SHADER" ] || [ ! -f "$FRAGMENT_SHADER" ]; then
    echo "Error: Shader source files not found."
    echo "  Expected: $VERTEX_SHADER"
    echo "  Expected: $FRAGMENT_SHADER"
    exit 1
fi

# 3. Ensure the output directory exists
mkdir -p "$OUT_DIR"

echo "Compiling shaders to SPIR-V with $COMPILER..."

# We use -S <stage> to ensure compatibility with older versions of glslangValidator.
# -G generates SPIR-V for OpenGL, which allows for loose uniforms.
$COMPILER -G -S vert "$VERTEX_SHADER" -o "$OUT_DIR/main.vert.spv" && echo "  [OK] $VERTEX_SHADER -> $OUT_DIR/main.vert.spv"
$COMPILER -G -S frag "$FRAGMENT_SHADER" -o "$OUT_DIR/main.frag.spv" && echo "  [OK] $FRAGMENT_SHADER -> $OUT_DIR/main.frag.spv"

echo "Shader compilation completed successfully."
