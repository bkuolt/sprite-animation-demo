#include "glad/gl.h"
#include "KHR/khrplatform.h"
#include <GLFW/glfw3.h>
#include <ktx.h>

#include <spdlog/spdlog.h>

constexpr GLenum GetGlInternalFormat(ktx_transcode_fmt_e format)
{
    switch (format)
    {
    case KTX_TTF_BC1_RGB:
        return GL_COMPRESSED_RGB_S3TC_DXT1_EXT; // DXT1
    case KTX_TTF_BC3_RGBA:
        return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; // DXT5
    case KTX_TTF_BC4_R:
        return GL_COMPRESSED_RED_RGTC1; // RGTC Red
    case KTX_TTF_BC5_RG:
        return GL_COMPRESSED_RG_RGTC2; // RGTC Red-Green
    case KTX_TTF_RGBA32:
        return GL_RGBA8; // Unkomprimiert 8-Bit
    case KTX_TTF_RGB565:
        return GL_RGB565; // Unkomprimiert 16-Bit
    default:
        throw std::runtime_error("Unsupported target format for GL mapping");
    }
}
GLuint UploadArray(ktxTexture2 *_texture, ktx_transcode_fmt_e _targetFormat)
{
    if (!_texture)
    {
        throw std::runtime_error("Texture is null before upload!");
    }

    // Cast auf die C-Basisklasse für die Daten-Offsets
    ktxTexture *baseTexture = ktxTexture(_texture);

    // Wir mappen das Format jetzt selbst!
    const GLenum internalFormat = GetGlInternalFormat(_targetFormat);
    const bool isCompressed = (_targetFormat != KTX_TTF_RGBA32 && _targetFormat != KTX_TTF_RGB565);

    GLuint textureId = 0;
    // 1. DSA: Textur direkt als Array erstellen
    glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &textureId);

    // 2. Filter setzen (Mipmap, falls vorhanden)
    glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, baseTexture->numLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // KTX2 speichert numLayers manchmal als 0, wenn es technisch kein Array ist.
    // Für einen 2D_ARRAY Upload brauchen wir aber mindestens Tiefe 1.
    const uint32_t numLayers = std::max(1u, baseTexture->numLayers);

    // 3. Immutable Storage allokieren (Parameter: ID, Mips, Format, Breite, Höhe, Tiefe/Layer)
    glTextureStorage3D(textureId, baseTexture->numLevels, internalFormat, baseTexture->baseWidth, baseTexture->baseHeight, numLayers);

    const uint8_t *baseData = ktxTexture_GetData(baseTexture);

    // 4. Mips und Layer iterieren und hochladen
    for (uint32_t level = 0; level < baseTexture->numLevels; ++level)
    {
        const uint32_t width = std::max(1u, baseTexture->baseWidth >> level);
        const uint32_t height = std::max(1u, baseTexture->baseHeight >> level);

        for (uint32_t layer = 0; layer < numLayers; ++layer)
        {
            size_t offset = 0;
            ktxTexture_GetImageOffset(baseTexture, level, layer, 0, &offset);

            size_t imageSize = ktxTexture_GetImageSize(baseTexture, level);
            const void *data = baseData + offset;

            if (isCompressed)
            {
                // Komprimiert (BC1, BC3, etc.)
                glCompressedTextureSubImage3D(textureId, level, 0, 0, layer, width, height, 1, internalFormat, static_cast<GLsizei>(imageSize), data);
            }
            else
            {
                // Fallback unkomprimiert
                GLenum format = (_targetFormat == KTX_TTF_RGBA32) ? GL_RGBA : GL_RGB;
                GLenum type = (_targetFormat == KTX_TTF_RGBA32) ? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT_5_6_5;
                glTextureSubImage3D(textureId, level, 0, 0, layer, width, height, 1, format, type, data);
            }
        }
    }

    spdlog::info("Successfully uploaded KTX2 Array Texture to GL ID: {}", textureId);
    return textureId;
}