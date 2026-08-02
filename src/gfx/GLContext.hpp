// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include <glad/gl.h>

namespace bgl
{
/**
 * @brief Initializes GLAD function pointers for OpenGL.
 */
void InitializeGLAD();

/**
 * @brief Sets up OpenGL debug logging, capabilities, and state options.
 */
void IntitializeOpenGL();

/**
 * @brief Queries current estimated VRAM usage in megabytes.
 */
[[nodiscard]] int GetVRAMUsageMB();
} // namespace bgl
