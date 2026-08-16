// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include <glad/gl.h>
#include <optional>

namespace bgl::gl
{
/**
 * @brief Initializes GLAD function pointers for OpenGL.
 */
void InitializeGLAD();

/**
 * @brief Sets up OpenGL debug logging, capabilities, and state options.
 */
void InitializeOpenGL();

/**
 * @brief Queries current estimated VRAM usage in megabytes.
 * @return VRAM usage in MB, or std::nullopt if the driver does not support the query.
 */
[[nodiscard]] std::optional<int> GetVRAMUsageMB();
} // namespace bgl::gl
