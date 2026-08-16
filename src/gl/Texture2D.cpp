// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "Texture2D.hpp"
#include <algorithm>
#include <cmath>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace bgl::gl
{
Texture2D::Texture2D(const ImageLayer &image, bool generateMipmaps)
{
    if (image.data.empty())
    {
        throw std::runtime_error("Cannot create Texture2D with empty image data");
    }

    const uint32_t width = image.width;
    const uint32_t height = image.height;
    const uint32_t channels = image.channels;

    const GLenum format = (channels == 3) ? GL_RGB : GL_RGBA;
    const GLenum internalFormat = (channels == 3) ? GL_RGB8 : GL_RGBA8;

    const GLsizei mipLevels =
        generateMipmaps ? static_cast<GLsizei>(std::floor(std::log2(std::max(width, height)))) + 1 : 1;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_handle);

    glTextureStorage2D(m_handle, mipLevels, internalFormat, width, height);
    glTextureSubImage2D(m_handle, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, image.data.data());

    if (generateMipmaps)
    {
        glGenerateTextureMipmap(m_handle);
    }

    spdlog::info("Created Texture2D ({}x{}) with GL ID: {}", width, height, m_handle);
}

Texture2D::~Texture2D()
{
    cleanup();
}

Texture2D::Texture2D(Texture2D &&other) noexcept : m_handle(other.m_handle)
{
    other.m_handle = 0;
}

Texture2D &Texture2D::operator=(Texture2D &&other) noexcept
{
    if (this != &other)
    {
        cleanup();
        m_handle = other.m_handle;
        other.m_handle = 0;
    }
    return *this;
}

void Texture2D::cleanup() noexcept
{
    if (m_handle != 0)
    {
        glDeleteTextures(1, &m_handle);
        spdlog::info("Deleted Texture2D GL ID: {}", m_handle);
        m_handle = 0;
    }
}
} // namespace bgl::gl
