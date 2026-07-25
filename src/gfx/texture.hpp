// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#pragma once

#include <glad/gl.h>
#include <ktx.h>
#include <vector>
#include <cstdint>
#include <span>

[[nodiscard]] GLuint UploadArray(ktxTexture2 *texture, ktx_transcode_fmt_e targetFormat);

namespace bgl
{
    struct ImageLayer
    {
        uint32_t width{0};
        uint32_t height{0};
        uint32_t channels{4}; // 3 = RGB, 4 = RGBA
        std::vector<uint8_t> data;
    };

    [[nodiscard]] GLuint UploadRawArray(std::span<const ImageLayer> layers, bool generateMipmaps = true);
} // namespace bgl
