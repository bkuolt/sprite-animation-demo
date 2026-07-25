// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#include "ktx.hpp"
#include "../gfx/texture.hpp"

#include "glad/gl.h"

#include <spdlog/spdlog.h>
#include <unordered_set>
#include <string>
#include <filesystem>
#include <stdexcept>

namespace bgl::ktx
{
    constexpr const char *glErrorString(GLenum error) noexcept
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

    [[nodiscard]] ktx_transcode_fmt_e GetTextureFormat(
        const std::unordered_set<std::string> &extensions,
        bool hasAlpha,
        bool isNormalMap = false)
    {
        if (extensions.contains("GL_EXT_texture_compression_s3tc"))
        {
            return hasAlpha ? KTX_TTF_BC3_RGBA : KTX_TTF_BC1_RGB;
        }

        if (extensions.contains("GL_EXT_texture_compression_dxt1"))
        {
            return KTX_TTF_BC1_RGB;
        }

        if (extensions.contains("GL_EXT_texture_compression_rgtc"))
        {
            return isNormalMap ? KTX_TTF_BC5_RG : KTX_TTF_BC4_R;
        }

        return hasAlpha ? KTX_TTF_RGBA32 : KTX_TTF_RGB565;
    }

    Loader::Loader(const std::filesystem::path &path, ktx_transcode_fmt_e targetFormat)
        : _targetFormat(targetFormat)
    {
        try
        {
            load(path);
        }
        catch (...)
        {
            if (_texture)
            {
                ktxTexture2_Destroy(_texture);
                _texture = nullptr;
            }
            throw;
        }

        spdlog::info("KTX layers: {}, levels: {}", _texture->numLayers, _texture->numLevels);
    }

    Loader::~Loader()
    {
        if (_texture)
        {
            ktxTexture2_Destroy(_texture);
        }
    }

    GLuint Loader::upload()
    {
        return UploadArray(_texture, _targetFormat);
    }

    void Loader::load(const std::filesystem::path &path)
    {
        const KTX_error_code result = ktxTexture2_CreateFromNamedFile(
            path.string().c_str(),
            KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
            &_texture
        );

        if (result != KTX_SUCCESS)
        {
            throw std::runtime_error(fmt::format("Failed to load KTX file: {}", path.string()));
        }

        spdlog::info("Loaded KTX file: {}", path.string());

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
            throw std::runtime_error("Failed to transcode KTX Basis texture");
        }

        const auto uncompressedSize = _texture->dataSize;
        spdlog::info("Transcoded KTX texture - compressed: {} MB, uncompressed: {} MB",
                     compressedSize / (1024 * 1024),
                     uncompressedSize / (1024 * 1024));
    }

} // namespace bgl::ktx