// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#pragma once

#include <glad/gl.h>
#include <vector>
#include <cstdint>
#include <span>

namespace bgl::gfx
{
    struct ImageLayer
    {
        uint32_t width{0};
        uint32_t height{0};
        uint32_t channels{4}; // 3 = RGB, 4 = RGBA
        std::vector<uint8_t> data;
    };

    class Texture2D
    {
    public:
        Texture2D() = default;
        explicit Texture2D(const ImageLayer& image, bool generateMipmaps = true);
        ~Texture2D();

        Texture2D(const Texture2D&) = delete;
        Texture2D& operator=(const Texture2D&) = delete;

        Texture2D(Texture2D&& other) noexcept;
        Texture2D& operator=(Texture2D&& other) noexcept;

        [[nodiscard]] GLuint getHandle() const { return m_handle; }
        [[nodiscard]] bool isValid() const { return m_handle != 0; }

    private:
        void cleanup() noexcept;
        GLuint m_handle{0};
    };
} // namespace bgl::gfx
