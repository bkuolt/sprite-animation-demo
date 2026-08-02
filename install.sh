#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

set -euo pipefail

echo "=================================================="
echo "  Sprite Animation Engine: Environment Setup      "
echo "=================================================="

echo ">> [1/4] Installing system build dependencies..."
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    git \
    python3 \
    python3-pip \
    pipx \
    glslang-tools \
    libgl-dev \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libfontconfig1-dev \
    curl \
    bzip2

pipx ensurepath || true

echo ">> [2/5] Installing KTX-Software (toktx)..."
if ! command -v toktx &> /dev/null; then
    echo "   Downloading KTX-Software-4.4.2..."
    mkdir -p /tmp/ktx_install && cd /tmp/ktx_install
    curl -sL https://github.com/KhronosGroup/KTX-Software/releases/download/v4.4.2/KTX-Software-4.4.2-Linux-x86_64.tar.bz2 | tar xj
    mkdir -p "$HOME/.local/bin"
    mkdir -p "$HOME/.local/lib"
    cp KTX-Software-4.4.2-Linux-x86_64/bin/toktx "$HOME/.local/bin/"
    cp -d KTX-Software-4.4.2-Linux-x86_64/lib/libktx.* "$HOME/.local/lib/"
    cd - > /dev/null
    rm -rf /tmp/ktx_install
    
    # Ensure local lib is in LD_LIBRARY_PATH via bashrc if not present
    if ! grep -q "LD_LIBRARY_PATH.*\.local/lib" "$HOME/.bashrc"; then
        echo 'export LD_LIBRARY_PATH="$HOME/.local/lib:$LD_LIBRARY_PATH"' >> "$HOME/.bashrc"
    fi
    # Also set it for current script execution
    export LD_LIBRARY_PATH="$HOME/.local/lib:$LD_LIBRARY_PATH"
else
    echo "   toktx is already installed."
fi

echo ">> [3/5] Installing Conan Package Manager..."
if ! command -v conan &> /dev/null; then
    pipx install conan
else
    echo "   Conan is already installed."
fi

if ! conan profile show default &> /dev/null; then
    echo "   Detecting default Conan profile..."
    conan profile detect
fi

echo ">> [4/5] Installing Task Runner..."
if ! command -v task &> /dev/null; then
    echo "   Installing Task runner to ~/.local/bin..."
    mkdir -p "$HOME/.local/bin"
    sh -c "$(curl -ssL https://taskfile.dev/install.sh)" -- -b "$HOME/.local/bin"
else
    echo "   Task runner is already installed."
fi

echo ">> [5/5] Fetching C++ Dependencies via Task..."
export PATH="$HOME/.local/bin:$HOME/.local/pipx/venvs/conan/bin:$PATH"
task setup

echo "=================================================="
echo "  Setup Complete!                                 "
echo "  You can now build and run the engine using:     "
echo "    $ task build                                  "
echo "    $ task run                                    "
echo "=================================================="
