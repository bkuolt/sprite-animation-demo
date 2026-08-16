// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include <glad/gl.h>
#include <ktx.h>
#include "GLHandle.hpp"

namespace bgl::gl
{
/**
 * @brief Represents an OpenGL Cube Map Texture.
 */
class TextureCube
{
  public:
    TextureCube() = default;

    /**
     * @brief Constructs a TextureCube from a KTX texture.
     * @param texture Pointer to the KTX texture object.
     * @param targetFormat The target transcoded format.
     */
    explicit TextureCube(ktxTexture2 *texture, ktx_transcode_fmt_e targetFormat);

    ~TextureCube() = default;

    TextureCube(const TextureCube &) = delete;
    TextureCube &operator=(const TextureCube &) = delete;

    TextureCube(TextureCube &&) noexcept = default;
    TextureCube &operator=(TextureCube &&) noexcept = default;

    /**
     * @brief Gets the underlying OpenGL texture handle.
     */
    [[nodiscard]] GLuint getHandle() const { return m_handle.get(); }
    [[nodiscard]] bool isValid() const { return static_cast<bool>(m_handle); }

  private:
    TextureHandle m_handle;
};
} // namespace bgl::gl
