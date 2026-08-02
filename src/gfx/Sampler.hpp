// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include <glad/gl.h>

namespace bgl::gfx
{
/**
 * @brief Wrapper for an OpenGL Sampler Object.
 *
 * Provides Direct State Access (DSA) for texture sampling configuration,
 * decoupling filtering and wrapping state from the Texture Objects themselves.
 */
class Sampler
{
  public:
    /**
     * @brief Creates a new OpenGL Sampler object.
     */
    Sampler();

    /**
     * @brief Destroys the Sampler object and frees GPU resources.
     */
    ~Sampler();

    // Prevent accidental copying
    Sampler(const Sampler &) = delete;
    Sampler &operator=(const Sampler &) = delete;

    // Strict move semantics
    Sampler(Sampler &&other) noexcept;
    Sampler &operator=(Sampler &&other) noexcept;

    /**
     * @brief Sets the magnification and minification filters.
     * @param minFilter The minification filter (e.g., GL_LINEAR_MIPMAP_LINEAR).
     * @param magFilter The magnification filter (e.g., GL_LINEAR).
     */
    void setFilter(GLenum minFilter, GLenum magFilter);

    /**
     * @brief Sets the texture wrapping modes.
     * @param wrapS The wrap mode for the S coordinate.
     * @param wrapT The wrap mode for the T coordinate.
     * @param wrapR The wrap mode for the R coordinate (default GL_CLAMP_TO_EDGE).
     */
    void setWrap(GLenum wrapS, GLenum wrapT, GLenum wrapR = GL_CLAMP_TO_EDGE);

    /**
     * @brief Sets the maximum anisotropy for filtering.
     * @param maxAnisotropy The maximum number of samples.
     */
    void setAnisotropy(float maxAnisotropy);

    /**
     * @brief Binds this sampler to the specified texture unit.
     * @param textureUnit The zero-based texture unit index.
     */
    void bind(GLuint textureUnit) const;

    /**
     * @brief Gets the underlying OpenGL handle.
     * @return The OpenGL sampler ID.
     */
    [[nodiscard]] GLuint getHandle() const
    {
        return m_handle;
    }

  private:
    void cleanup() noexcept;
    GLuint m_handle{0};
};
} // namespace bgl::gfx
