// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "KtxLoader.hpp"
#include "../gl/Texture2DArray.hpp"

#include <filesystem>
#include <glad/gl.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace bgl::io
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

[[nodiscard]] ktx_transcode_fmt_e GetTextureFormat(const std::unordered_set<std::string> &extensions, bool hasAlpha,
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

KtxLoader::KtxLoader(const std::filesystem::path &path, ktx_transcode_fmt_e targetFormat) : _targetFormat(targetFormat)
{
    load(path);
    transcode();
}

KtxLoader::KtxLoader(std::span<const std::byte> memoryBuffer, ktx_transcode_fmt_e targetFormat) : _targetFormat(targetFormat)
{
    KTX_error_code result = ktxTexture2_CreateFromMemory(
        reinterpret_cast<const ktx_uint8_t *>(memoryBuffer.data()),
        memoryBuffer.size(),
        KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
        &_texture);

    if (result != KTX_SUCCESS)
    {
        throw std::runtime_error(fmt::format("Failed to load KTX from memory. Error: {}", ktxErrorString(result)));
    }
    transcode();
}

KtxLoader::~KtxLoader()
{
    if (_texture)
    {
        ktxTexture_Destroy(ktxTexture(_texture));
        _texture = nullptr;
    }
}

void KtxLoader::load(const std::filesystem::path &path)
{
    if (!std::filesystem::exists(path))
    {
        throw std::runtime_error(fmt::format("KTX file not found: {}", path.string()));
    }

    KTX_error_code result =
        ktxTexture2_CreateFromNamedFile(path.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &_texture);

    if (result != KTX_SUCCESS)
    {
        throw std::runtime_error(
            fmt::format("Failed to load KTX file: {}. Error: {}", path.string(), ktxErrorString(result)));
    }

    spdlog::info("Loaded KTX file: {}", path.string());
}

void KtxLoader::transcode()
{
    if (!_texture)
        return;

    if (ktxTexture2_NeedsTranscoding(_texture))
    {
        KTX_error_code result = ktxTexture2_TranscodeBasis(_texture, _targetFormat, 0);

        if (result != KTX_SUCCESS)
        {
            throw std::runtime_error(fmt::format("Failed to transcode KTX texture. Error: {}", ktxErrorString(result)));
        }

        const auto uncompressedBytes = ktxTexture_GetDataSizeUncompressed(ktxTexture(_texture));
        const auto compressedBytes = ktxTexture_GetDataSize(ktxTexture(_texture));
        spdlog::info("Transcoded KTX texture - compressed: {} MB, uncompressed: {} MB", compressedBytes / (1024 * 1024),
                     uncompressedBytes / (1024 * 1024));
        spdlog::info("KTX layers: {}, levels: {}", _texture->numLayers, _texture->numLevels);
    }
}

std::unique_ptr<bgl::gl::Texture2DArray> KtxLoader::upload()
{
    return std::make_unique<bgl::gl::Texture2DArray>(_texture, _targetFormat);
}
} // namespace bgl::io