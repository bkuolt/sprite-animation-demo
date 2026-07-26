// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "font.hpp"
#include "textShaper.hpp"

#include <glad/gl.h>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <cstdint>
#include <string_view>
#include <vector>

namespace bgl
{
class TextTexture
{
  public:
    TextTexture() = default;
    TextTexture(GLuint textureId, uint32_t width, uint32_t height);
    ~TextTexture();

    TextTexture(const TextTexture &) = delete;
    TextTexture &operator=(const TextTexture &) = delete;

    TextTexture(TextTexture &&other) noexcept;
    TextTexture &operator=(TextTexture &&other) noexcept;

    [[nodiscard]] GLuint GetHandle() const
    {
        return m_textureId;
    }
    [[nodiscard]] uint32_t GetWidth() const
    {
        return m_width;
    }
    [[nodiscard]] uint32_t GetHeight() const
    {
        return m_height;
    }
    [[nodiscard]] bool IsValid() const
    {
        return m_textureId != 0;
    }

    void Bind(uint32_t slot = 0) const;

  private:
    void Cleanup() noexcept;

    GLuint m_textureId{0};
    uint32_t m_width{0};
    uint32_t m_height{0};
};

class TextRenderer
{
  public:
    [[nodiscard]] static TextTexture RenderToTexture(const Font &font, std::string_view text,
                                                     glm::u8vec4 textColor = {255, 255, 255, 255},
                                                     glm::u8vec4 backgroundColor = {0, 0, 0, 160},
                                                     uint32_t padding = 6);

    [[nodiscard]] static TextTexture RenderShapedToTexture(const Font &font, const ShapedText &shapedText,
                                                           glm::u8vec4 textColor = {255, 255, 255, 255},
                                                           glm::u8vec4 backgroundColor = {0, 0, 0, 160},
                                                           uint32_t padding = 6);
};
} // namespace bgl
