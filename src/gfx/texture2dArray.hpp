// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#pragma once

#include "texture2d.hpp" // For bgl::gfx::ImageLayer
#include <glad/gl.h>
#include <ktx.h>
#include <span>
#include <vector>

namespace bgl::gfx
{
class Texture2DArray
{
  public:
    Texture2DArray() = default;

    // Construct from KTX texture
    explicit Texture2DArray(ktxTexture2 *texture, ktx_transcode_fmt_e targetFormat);

    // Construct from raw image layers
    explicit Texture2DArray(std::span<const ImageLayer> layers, bool generateMipmaps = true);

    ~Texture2DArray();

    Texture2DArray(const Texture2DArray &) = delete;
    Texture2DArray &operator=(const Texture2DArray &) = delete;

    Texture2DArray(Texture2DArray &&other) noexcept;
    Texture2DArray &operator=(Texture2DArray &&other) noexcept;

    [[nodiscard]] GLuint getHandle() const
    {
        return m_handle;
    }
    [[nodiscard]] bool isValid() const
    {
        return m_handle != 0;
    }

    [[nodiscard]] GLuint release() noexcept
    {
        GLuint handle = m_handle;
        m_handle = 0;
        return handle;
    }

  private:
    void cleanup() noexcept;
    GLuint m_handle{0};
};
} // namespace bgl::gfx
