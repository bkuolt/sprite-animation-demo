// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#include "texture.hpp"

#include "glad/gl.h"
#include <ktx.h>

#include <spdlog/spdlog.h>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <span>

constexpr GLenum GetGlInternalFormat(ktx_transcode_fmt_e format)
{
    switch (format)
    {
    case KTX_TTF_BC1_RGB:
        return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
    case KTX_TTF_BC3_RGBA:
        return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    case KTX_TTF_BC4_R:
        return GL_COMPRESSED_RED_RGTC1;
    case KTX_TTF_BC5_RG:
        return GL_COMPRESSED_RG_RGTC2;
    case KTX_TTF_RGBA32:
        return GL_RGBA8;
    case KTX_TTF_RGB565:
        return GL_RGB565;
    default:
        throw std::runtime_error("Unsupported target format for GL mapping");
    }
}

GLuint UploadArray(ktxTexture2 *texture, ktx_transcode_fmt_e targetFormat)
{
    if (!texture)
    {
        throw std::runtime_error("Texture is null before upload!");
    }

    auto *baseTexture = ktxTexture(texture);

    const GLenum internalFormat = GetGlInternalFormat(targetFormat);
    const bool isCompressed = (targetFormat != KTX_TTF_RGBA32 && targetFormat != KTX_TTF_RGB565);

    GLuint textureId = 0;
    glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &textureId);

    glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, baseTexture->numLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    const uint32_t numLayers = std::max(1u, baseTexture->numLayers);

    glTextureStorage3D(textureId, baseTexture->numLevels, internalFormat, baseTexture->baseWidth, baseTexture->baseHeight, numLayers);

    const uint8_t *baseData = ktxTexture_GetData(baseTexture);

    for (uint32_t level = 0; level < baseTexture->numLevels; ++level)
    {
        const uint32_t width = std::max(1u, baseTexture->baseWidth >> level);
        const uint32_t height = std::max(1u, baseTexture->baseHeight >> level);

        for (uint32_t layer = 0; layer < numLayers; ++layer)
        {
            size_t offset = 0;
            ktxTexture_GetImageOffset(baseTexture, level, layer, 0, &offset);

            const size_t imageSize = ktxTexture_GetImageSize(baseTexture, level);
            const void *data = baseData + offset;

            if (isCompressed)
            {
                glCompressedTextureSubImage3D(textureId, level, 0, 0, layer, width, height, 1, internalFormat, static_cast<GLsizei>(imageSize), data);
            }
            else
            {
                const GLenum format = (targetFormat == KTX_TTF_RGBA32) ? GL_RGBA : GL_RGB;
                const GLenum type = (targetFormat == KTX_TTF_RGBA32) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT_5_6_5;
                glTextureSubImage3D(textureId, level, 0, 0, layer, width, height, 1, format, type, data);
            }
        }
    }

    spdlog::info("Successfully uploaded KTX2 Array Texture to GL ID: {}", textureId);
    return textureId;
}

namespace bgl
{
    GLuint UploadRawArray(std::span<const ImageLayer> layers, bool generateMipmaps)
    {
        if (layers.empty())
        {
            throw std::runtime_error("Cannot upload empty layer list as texture array");
        }

        const uint32_t width = layers[0].width;
        const uint32_t height = layers[0].height;
        const uint32_t channels = layers[0].channels;
        const uint32_t numLayers = static_cast<uint32_t>(layers.size());

        const GLenum format = (channels == 3) ? GL_RGB : GL_RGBA;
        const GLenum internalFormat = (channels == 3) ? GL_RGB8 : GL_RGBA8;

        const GLsizei mipLevels = generateMipmaps 
            ? static_cast<GLsizei>(std::floor(std::log2(std::max(width, height)))) + 1 
            : 1;

        GLuint textureId = 0;
        glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &textureId);

        glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTextureStorage3D(textureId, mipLevels, internalFormat, width, height, numLayers);

        for (uint32_t layer = 0; layer < numLayers; ++layer)
        {
            if (layers[layer].data.empty())
            {
                throw std::runtime_error(fmt::format("Layer {} has empty data buffer", layer));
            }
            glTextureSubImage3D(textureId, 0, 0, 0, layer, width, height, 1, format, GL_UNSIGNED_BYTE, layers[layer].data.data());
        }

        if (generateMipmaps)
        {
            glGenerateTextureMipmap(textureId);
        }

        spdlog::info("Successfully uploaded Raw Image Array ({} layers, {}x{}) to GL ID: {}", numLayers, width, height, textureId);
        return textureId;
    }
} // namespace bgl
