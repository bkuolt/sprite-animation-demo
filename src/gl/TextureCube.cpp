// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "TextureCube.hpp"
#include <algorithm>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace
{
constexpr GLenum GetGlInternalFormat(ktx_transcode_fmt_e format)
{
    switch (format)
    {
    case KTX_TTF_BC1_RGB: return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
    case KTX_TTF_BC3_RGBA: return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    case KTX_TTF_BC4_R: return GL_COMPRESSED_RED_RGTC1;
    case KTX_TTF_BC5_RG: return GL_COMPRESSED_RG_RGTC2;
    case KTX_TTF_RGBA32: return GL_RGBA8;
    case KTX_TTF_RGB565: return GL_RGB565;
    default: throw std::runtime_error("Unsupported target format for GL mapping");
    }
}
}

namespace bgl::gl
{
TextureCube::TextureCube(ktxTexture2 *texture, ktx_transcode_fmt_e targetFormat)
{
    if (!texture) throw std::runtime_error("Texture is null before upload!");
    
    auto *baseTexture = ktxTexture(texture);
    if (!baseTexture->isCubemap) throw std::runtime_error("KTX texture is not a cubemap!");

    const GLenum internalFormat = GetGlInternalFormat(targetFormat);
    const bool isCompressed = (targetFormat != KTX_TTF_RGBA32 && targetFormat != KTX_TTF_RGB565);

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_handle);
    glTextureStorage2D(m_handle, baseTexture->numLevels, internalFormat, baseTexture->baseWidth, baseTexture->baseHeight);

    const uint8_t *baseData = ktxTexture_GetData(baseTexture);

    for (uint32_t level = 0; level < baseTexture->numLevels; ++level)
    {
        const uint32_t width = std::max(1u, baseTexture->baseWidth >> level);
        const uint32_t height = std::max(1u, baseTexture->baseHeight >> level);

        for (uint32_t face = 0; face < 6; ++face)
        {
            size_t offset = 0;
            ktxTexture_GetImageOffset(baseTexture, level, 0, face, &offset);
            const size_t imageSize = ktxTexture_GetImageSize(baseTexture, level);
            const void *data = baseData + offset;

            if (isCompressed)
            {
                glCompressedTextureSubImage3D(m_handle, level, 0, 0, face, width, height, 1, internalFormat, static_cast<GLsizei>(imageSize), data);
            }
            else
            {
                const GLenum format = (targetFormat == KTX_TTF_RGBA32) ? GL_RGBA : GL_RGB;
                const GLenum type = (targetFormat == KTX_TTF_RGBA32) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT_5_6_5;
                glTextureSubImage3D(m_handle, level, 0, 0, face, width, height, 1, format, type, data);
            }
        }
    }

    spdlog::info("Created TextureCube (KTX2) with GL ID: {}", m_handle);
}

TextureCube::~TextureCube() { cleanup(); }

TextureCube::TextureCube(TextureCube &&other) noexcept : m_handle(other.m_handle) { other.m_handle = 0; }

TextureCube &TextureCube::operator=(TextureCube &&other) noexcept
{
    if (this != &other)
    {
        cleanup();
        m_handle = other.m_handle;
        other.m_handle = 0;
    }
    return *this;
}

void TextureCube::cleanup() noexcept
{
    if (m_handle != 0)
    {
        glDeleteTextures(1, &m_handle);
        m_handle = 0;
    }
}
} // namespace bgl::gl
