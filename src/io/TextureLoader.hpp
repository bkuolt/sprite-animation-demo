// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include <glad/gl.h>

#include "../gfx/Texture2DArray.hpp"
#include <filesystem>
#include <memory>
#include <span>
#include <string_view>
#include <cstddef>

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

/**
 * @brief Automatically loads a texture from disk by detecting magic numbers or file extension.
 * Supports KTX2, PNG, and JPEG formats.
 */
[[nodiscard]] std::unique_ptr<bgl::gfx::Texture2DArray> loadTexture(const std::filesystem::path &path);

/**
 * @brief Automatically loads a texture from raw memory by inspecting magic bytes or using a format extension hint.
 */
[[nodiscard]] std::unique_ptr<bgl::gfx::Texture2DArray> loadTexture(std::span<const std::byte> memoryBuffer,
                                                                      std::string_view extensionHint = "");
} // namespace bgl::io
