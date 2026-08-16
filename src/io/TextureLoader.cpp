// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "TextureLoader.hpp"
#include "KtxLoader.hpp"

#ifdef BGL_ENABLE_PNG_LOADER
#include "PngLoader.hpp"
#endif

#ifdef BGL_ENABLE_JPEG_LOADER
#include "JpegLoader.hpp"
#endif

#include <spdlog/spdlog.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstring>

namespace bgl::io
{
namespace
{
enum class TextureFormat
{
    Unknown,
    KTX2,
    PNG,
    JPEG
};

TextureFormat detectFormatFromMagic(std::span<const std::byte> data)
{
    if (data.size() >= 12)
    {
        const static std::uint8_t ktx2Magic[12] = {0xAB, 'K', 'T', 'X', ' ', '2', '0', 0xBB, '\r', '\n', 0x1A, '\n'};
        if (std::memcmp(data.data(), ktx2Magic, 12) == 0)
        {
            return TextureFormat::KTX2;
        }
    }

    if (data.size() >= 8)
    {
        const static std::uint8_t pngMagic[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
        if (std::memcmp(data.data(), pngMagic, 8) == 0)
        {
            return TextureFormat::PNG;
        }
    }

    if (data.size() >= 3)
    {
        const static std::uint8_t jpegMagic[3] = {0xFF, 0xD8, 0xFF};
        if (std::memcmp(data.data(), jpegMagic, 3) == 0)
        {
            return TextureFormat::JPEG;
        }
    }

    return TextureFormat::Unknown;
}

TextureFormat detectFormatFromExtension(std::string_view ext) noexcept
{
    // Case-insensitive comparison without allocating a temporary string.
    auto iequals = [](std::string_view a, std::string_view b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i)
            if (std::tolower(static_cast<unsigned char>(a[i])) !=
                std::tolower(static_cast<unsigned char>(b[i]))) return false;
        return true;
    };

    if (iequals(ext, ".ktx2") || iequals(ext, "ktx2")) return TextureFormat::KTX2;
    if (iequals(ext, ".png")  || iequals(ext, "png"))  return TextureFormat::PNG;
    if (iequals(ext, ".jpg")  || iequals(ext, "jpg")  ||
        iequals(ext, ".jpeg") || iequals(ext, "jpeg")) return TextureFormat::JPEG;

    return TextureFormat::Unknown;
}
} // namespace

std::unique_ptr<bgl::gl::Texture2DArray> loadTexture(const std::filesystem::path &path)
{
    if (!std::filesystem::exists(path))
    {
        spdlog::error("Texture file does not exist: {}", path.string());
        return nullptr;
    }

    std::ifstream file(path, std::ios::binary);
    std::vector<std::byte> header(16);
    file.read(reinterpret_cast<char *>(header.data()), header.size());
    const auto bytesRead = file.gcount();
    header.resize(static_cast<size_t>(bytesRead));

    TextureFormat format = detectFormatFromMagic(header);
    if (format == TextureFormat::Unknown)
    {
        format = detectFormatFromExtension(path.extension().string());
    }

    switch (format)
    {
    case TextureFormat::KTX2:
    {
        KtxLoader loader(path, KTX_TTF_BC3_RGBA);
        return loader.upload();
    }
#ifdef BGL_ENABLE_PNG_LOADER
    case TextureFormat::PNG:
    {
        PngLoader loader(path);
        return loader.upload();
    }
#endif
#ifdef BGL_ENABLE_JPEG_LOADER
    case TextureFormat::JPEG:
    {
        JpegLoader loader(path);
        return loader.upload();
    }
#endif
    default:
        spdlog::error("Unsupported texture format for file: {}", path.string());
        return nullptr;
    }
}

std::unique_ptr<bgl::gl::Texture2DArray> loadTexture(std::span<const std::byte> memoryBuffer, std::string_view extensionHint)
{
    TextureFormat format = detectFormatFromMagic(memoryBuffer);
    if (format == TextureFormat::Unknown && !extensionHint.empty())
    {
        format = detectFormatFromExtension(extensionHint);
    }

    switch (format)
    {
    case TextureFormat::KTX2:
    {
        KtxLoader loader(memoryBuffer, KTX_TTF_BC3_RGBA);
        return loader.upload();
    }
#ifdef BGL_ENABLE_PNG_LOADER
    case TextureFormat::PNG:
    {
        PngLoader loader(memoryBuffer);
        return loader.upload();
    }
#endif
#ifdef BGL_ENABLE_JPEG_LOADER
    case TextureFormat::JPEG:
    {
        JpegLoader loader(memoryBuffer);
        return loader.upload();
    }
#endif
    default:
        spdlog::error("Unsupported or unrecognized raw texture memory buffer format.");
        return nullptr;
    }
}
} // namespace bgl::io
