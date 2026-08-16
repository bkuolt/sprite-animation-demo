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

    ~TextureCube();

    TextureCube(const TextureCube &) = delete;
    TextureCube &operator=(const TextureCube &) = delete;

    TextureCube(TextureCube &&other) noexcept;
    TextureCube &operator=(TextureCube &&other) noexcept;

    /**
     * @brief Gets the underlying OpenGL texture handle.
     */
    [[nodiscard]] GLuint getHandle() const { return m_handle; }
    [[nodiscard]] bool isValid() const { return m_handle != 0; }

  private:
    void cleanup() noexcept;
    GLuint m_handle{0};
};
} // namespace bgl::gl
