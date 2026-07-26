// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#pragma once

#include "glad/gl.h"
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace bgl::io
{
#ifdef BGL_ENABLE_GLSL_LOADER
[[nodiscard]] std::string LoadShaderFromFile(const std::filesystem::path &filepath);
#endif
[[nodiscard]] std::vector<uint32_t> LoadSPIRVShaderFromFile(const std::filesystem::path &filepath);
#ifdef BGL_ENABLE_GLSL_LOADER
[[nodiscard]] GLuint CreateShaderProgramFromGLSL(std::string_view vertexSrc, std::string_view fragmentSrc);
#endif
[[nodiscard]] GLuint CreateShaderProgramFromSPIRV(std::span<const uint32_t> vertexSpv,
                                                  std::span<const uint32_t> fragmentSpv);
} // namespace bgl::io
