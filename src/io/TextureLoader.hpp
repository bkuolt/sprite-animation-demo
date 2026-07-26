// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#pragma once

#include <glad/gl.h>

#include "../gfx/Texture2DArray.hpp"
#include <memory>

namespace bgl::io
{
/**
 * @brief Interface for polymorphic texture loaders.
 */
class ITextureLoader
{
  public:
    virtual ~ITextureLoader() = default;

    /**
     * @brief Uploads loaded texture data to the GPU.
     * @return Unique pointer to the created Texture2DArray.
     */
    [[nodiscard]] virtual std::unique_ptr<bgl::gfx::Texture2DArray> upload() = 0;
};
} // namespace bgl::io
