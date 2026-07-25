#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2024-2026 Bastian. All rights reserved.

set -euo pipefail

echo "=================================================="
echo "  Setting up Sprite Animation Demo Dependencies   "
echo "=================================================="

# 1. Install system dependencies & development headers
echo "[1/4] Installing system build packages..."
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
    libxi-dev

pipx ensurepath || true

# 2. Install Conan package manager
echo "[2/4] Installing Conan 2.x..."
if ! command -v conan &> /dev/null; then
    pipx install conan
else
    echo "Conan is already installed."
fi

# Detect default Conan profile if not present
if ! conan profile show default &> /dev/null; then
    echo "Detecting default Conan profile..."
    conan profile detect
fi

# 3. Install Task runner if not available
echo "[3/4] Checking Task runner..."
if ! command -v task &> /dev/null; then
    echo "Installing Task runner to ~/.local/bin..."
    mkdir -p "$HOME/.local/bin"
    sh -c "$(curl -ssL https://taskfile.dev/install.sh)" -- -b "$HOME/.local/bin"
else
    echo "Task runner is already installed."
fi

# 4. Fetch C++ dependencies via Conan
echo "[4/4] Setting up C++ dependencies via Task..."
export PATH="$HOME/.local/bin:$HOME/.local/pipx/venvs/conan/bin:$PATH"
task setup

echo "=================================================="
echo "  Setup Complete! You can now build and run:      "
echo "    task build                                    "
echo "    task run                                      "
echo "=================================================="
