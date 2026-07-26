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
    libfontconfig1-dev

pipx ensurepath || true

echo ">> [2/4] Installing Conan Package Manager..."
if ! command -v conan &> /dev/null; then
    pipx install conan
else
    echo "   Conan is already installed."
fi

if ! conan profile show default &> /dev/null; then
    echo "   Detecting default Conan profile..."
    conan profile detect
fi

echo ">> [3/4] Installing Task Runner..."
if ! command -v task &> /dev/null; then
    echo "   Installing Task runner to ~/.local/bin..."
    mkdir -p "$HOME/.local/bin"
    sh -c "$(curl -ssL https://taskfile.dev/install.sh)" -- -b "$HOME/.local/bin"
else
    echo "   Task runner is already installed."
fi

echo ">> [4/4] Fetching C++ Dependencies via Task..."
export PATH="$HOME/.local/bin:$HOME/.local/pipx/venvs/conan/bin:$PATH"
task setup

echo "=================================================="
echo "  Setup Complete!                                 "
echo "  You can now build and run the engine using:     "
echo "    $ task build                                  "
echo "    $ task run                                    "
echo "=================================================="
