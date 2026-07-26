#!/bin/bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2024-2026 Bastian. All rights reserved.

set -e

COMPILER="glslangValidator"
SRC_DIR="src/shaders"
OUT_DIR="assets"

if ! command -v $COMPILER &> /dev/null; then
    echo "Error: '$COMPILER' not found."
    echo "Please install 'glslang-tools' (e.g., with 'sudo apt install glslang-tools')."
    exit 1
fi

mkdir -p "$OUT_DIR"

echo "Compiling GLSL shaders in $SRC_DIR to SPIR-V..."

$COMPILER -G -S vert "$SRC_DIR/main.vs" -o "$OUT_DIR/main.vert.spv" && echo "  [OK] $SRC_DIR/main.vs -> $OUT_DIR/main.vert.spv"
$COMPILER -G -S frag "$SRC_DIR/main.fs" -o "$OUT_DIR/main.frag.spv" && echo "  [OK] $SRC_DIR/main.fs -> $OUT_DIR/main.frag.spv"

$COMPILER -G -S vert "$SRC_DIR/text.vs" -o "$OUT_DIR/text.vert.spv" && echo "  [OK] $SRC_DIR/text.vs -> $OUT_DIR/text.vert.spv"
$COMPILER -G -S frag "$SRC_DIR/text.fs" -o "$OUT_DIR/text.frag.spv" && echo "  [OK] $SRC_DIR/text.fs -> $OUT_DIR/text.frag.spv"

echo "Shader compilation completed successfully."
