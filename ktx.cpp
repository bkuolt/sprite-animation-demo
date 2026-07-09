#include "ktx.hpp"

#include "glad/gl.h" // KRITISCH: Muss vor ktx.h kommen, damit glGenTextures etc. bekannt sind!
#include <ktx.h>

#include <spdlog/spdlog.h>
#include <unordered_set>
#include <string>
#include <filesystem>
#include <stdexcept>

namespace bgl::ktx
{
    constexpr const char *glErrorString(GLenum error)
    {
        switch (error)
        {
        case GL_NO_ERROR:
            return "GL_NO_ERROR";
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        default:
            return "UNKNOWN_GL_ERROR";
        }
    }

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

    ktx_transcode_fmt_e GetTextureFormat(
        const std::unordered_set<std::string> &extensions,
        bool hasAlpha,
        bool isNormalMap = false)
    {
        // 1. S3TC (DXT1, DXT3, DXT5 / BC1, BC2, BC3)
        if (extensions.contains("GL_EXT_texture_compression_s3tc"))
        {
            return hasAlpha ? KTX_TTF_BC3_RGBA : KTX_TTF_BC1_RGB;
        }

        // 2. DXT1 explizit (BC1)
        if (extensions.contains("GL_EXT_texture_compression_dxt1"))
        {
            return KTX_TTF_BC1_RGB;
        }

        // 3. RGTC (BC4, BC5)
        // Optimal für reine Daten-Texturen, um S3TC-Artefakte zu vermeiden.
        if (extensions.contains("GL_EXT_texture_compression_rgtc"))
        {
            return isNormalMap ? KTX_TTF_BC5_RG : KTX_TTF_BC4_R;
        }

        // 4. Absoluter Fallback (unkomprimiert)
        return hasAlpha ? KTX_TTF_RGBA32 : KTX_TTF_RGB565;
    }

    Loader::Loader(const std::filesystem::path &path, ktx_transcode_fmt_e targetFormat)
        : _targetFormat(targetFormat)
    {
        try
        {
            load(path);
        }
        catch (const std::exception &e)
        {
            ktxTexture2_Destroy(_texture);
            throw;
        }

        // print num  layers, mipmaps yes, no
        spdlog::info("num layers: {}", _texture->numLayers);
        spdlog::info("num levels: {}", _texture->numLevels);
    }

    Loader::~Loader()
    {
        ktxTexture2_Destroy(_texture);
    }

//[2026-07-09 12:27:49.257] [info] num layers: 13
//[2026-07-09 12:27:49.257] [info] num levels: 10
//Could not load OpenGL command: glBindTexture!
//[2026-07-09 12:27:49.284] [error] failed to upload KTX texture. KTX error: Metadata key or loader-required GPU function not found., GL error: GL_NO_ERROR
//task: Failed to run task "run": exit status 1

    GLuint Loader::upload()
    {

        return uploadArray();

        GLuint glTextureId = 0;
        GLenum target = 0;
        GLenum glError = 0;

        const KTX_error_code result = ktxTexture_GLUpload(ktxTexture(_texture), &glTextureId, &target, &glError);
        if (result != KTX_SUCCESS)
        {
            auto message = std::format("failed to upload KTX texture. KTX error: {}, GL error: {}", ktxErrorString(result), glErrorString(glError));
            throw std::runtime_error(message);
        }

        return glTextureId;
    }

    GLuint Loader::uploadArray()
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

    void Loader::load(const std::filesystem::path &path)
    {
        KTX_error_code result = ktxTexture2_CreateFromNamedFile(path.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &_texture);
        if (result != KTX_SUCCESS)
        {
            throw std::runtime_error("failed to load KTX file");
        }

        spdlog::info("loaded KTX file: {}", path.string());

        if (ktxTexture2_NeedsTranscoding(_texture))
        {
            transcode();
        }
    }

    void Loader::transcode()
    {
        const size_t compressedSize = _texture->dataSize;

        const auto result = ktxTexture2_TranscodeBasis(_texture, _targetFormat, 0);
        if (result != KTX_SUCCESS)
        {
            throw std::runtime_error("failed to transcode KTX file");
        }

        const auto uncompressedSize = _texture->dataSize;
        spdlog::info("transcoded KTX file");
        spdlog::info("  compressed size: {} MB", compressedSize / (1024 * 1024));
        spdlog::info("uncompressed size: {} MB", uncompressedSize / (1024 * 1024));
    }

} // bgl::ktx