// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "GLContext.hpp"
#include "QuadMesh.hpp"
#include "Sampler.hpp"
#include <memory>

namespace bgl
{
/**
 * @brief Creates a default texture sampler with anisotropic filtering.
 */
[[nodiscard]] std::unique_ptr<gfx::Sampler> CreateDefaultSampler();
} // namespace bgl
