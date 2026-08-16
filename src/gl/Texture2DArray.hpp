// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "Texture2D.hpp" // For bgl::gfx::ImageLayer
#include <glad/gl.h>
#include <ktx.h>
#include <span>
#include "GLHandle.hpp"

namespace bgl::gl
{
/**
 * @brief Represents an OpenGL 2D Texture Array Object.
 * Utilizes Direct State Access (DSA) for uploading image layers.
 */
class Texture2DArray
{
  public:
    Texture2DArray() = default;

    /**
     * @brief Constructs a Texture2DArray from a KTX texture.
     * @param texture Pointer to the KTX texture object.
     * @param targetFormat The target transcoded format.
     */
    explicit Texture2DArray(ktxTexture2 *texture, ktx_transcode_fmt_e targetFormat);

    /**
     * @brief Constructs a Texture2DArray from raw image layers.
     * @param layers A span of image layers.
     * @param generateMipmaps Whether to generate mipmaps automatically.
     */
    explicit Texture2DArray(std::span<const ImageLayer> layers, bool generateMipmaps = true);

    ~Texture2DArray();

    Texture2DArray(const Texture2DArray &) = delete;
    Texture2DArray &operator=(const Texture2DArray &) = delete;

    Texture2DArray(Texture2DArray &&other) noexcept;
    Texture2DArray &operator=(Texture2DArray &&other) noexcept;

    /**
     * @brief Gets the underlying OpenGL texture handle.
     * @return The OpenGL texture ID.
     */
    [[nodiscard]] GLuint getHandle() const
    {
        return m_handle;
    }

    /**
     * @brief Checks if the texture is valid.
     * @return true if the texture handle is not 0.
     */
    [[nodiscard]] bool isValid() const
    {
        return m_handle != 0;
    }

    /**
     * @brief Returns the number of array layers (animation frames) in the texture.
     */
    [[nodiscard]] uint32_t layerCount() const noexcept
    {
        return m_layerCount;
    }

    /**
     * @brief Releases ownership of the texture handle.
     * @return The OpenGL texture ID.
     */
    [[nodiscard]] GLuint release() noexcept
    {
        GLuint handle = m_handle;
        m_handle = 0;
        return handle;
    }

  private:
    void cleanup() noexcept;
    GLuint   m_handle{0};
    uint32_t m_layerCount{0};
};
} // namespace bgl::gl
