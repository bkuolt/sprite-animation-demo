// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include <cstdint>
#include <glad/gl.h>
#include <span>
#include <vector>

namespace bgl::gfx
{
/**
 * @brief Represents a single layer of an image.
 */
struct ImageLayer
{
    uint32_t width{0};
    uint32_t height{0};
    uint32_t channels{4}; // 3 = RGB, 4 = RGBA
    std::vector<uint8_t> data;
};

/**
 * @brief Represents an OpenGL 2D Texture Object.
 * Utilizes Direct State Access (DSA).
 */
class Texture2D
{
  public:
    Texture2D() = default;

    /**
     * @brief Constructs a Texture2D from an image layer.
     * @param image The image data.
     * @param generateMipmaps Whether to generate mipmaps automatically.
     */
    explicit Texture2D(const ImageLayer &image, bool generateMipmaps = true);
    ~Texture2D();

    Texture2D(const Texture2D &) = delete;
    Texture2D &operator=(const Texture2D &) = delete;

    Texture2D(Texture2D &&other) noexcept;
    Texture2D &operator=(Texture2D &&other) noexcept;

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

  private:
    void cleanup() noexcept;
    GLuint m_handle{0};
};
} // namespace bgl::gfx
