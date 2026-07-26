// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#include "Text_renderer.hpp"

#include <spdlog/spdlog.h>
#include <algorithm>
#include <stdexcept>
#include <utility>

namespace bgl
{
    TextTexture::TextTexture(GLuint textureId, uint32_t width, uint32_t height)
        : m_textureId(textureId), m_width(width), m_height(height)
    {
    }

    TextTexture::~TextTexture()
    {
        Cleanup();
    }

    TextTexture::TextTexture(TextTexture &&other) noexcept
        : m_textureId(other.m_textureId), m_width(other.m_width), m_height(other.m_height)
    {
        other.m_textureId = 0;
        other.m_width = 0;
        other.m_height = 0;
    }

    TextTexture &TextTexture::operator=(TextTexture &&other) noexcept
    {
        if (this != &other)
        {
            Cleanup();

            m_textureId = other.m_textureId;
            m_width = other.m_width;
            m_height = other.m_height;

            other.m_textureId = 0;
            other.m_width = 0;
            other.m_height = 0;
        }
        return *this;
    }

    void TextTexture::Cleanup() noexcept
    {
        if (m_textureId != 0)
        {
            glDeleteTextures(1, &m_textureId);
            m_textureId = 0;
            m_width = 0;
            m_height = 0;
        }
    }

    void TextTexture::Bind(uint32_t slot) const
    {
        if (m_textureId != 0)
        {
            glBindTextureUnit(slot, m_textureId);
        }
    }

    TextTexture TextRenderer::RenderToTexture(
        const Font &font,
        std::string_view text,
        glm::u8vec4 textColor,
        glm::u8vec4 backgroundColor,
        uint32_t padding)
    {
        ShapedText shapedText = TextShaper::ShapeText(font, text);
        return RenderShapedToTexture(font, shapedText, textColor, backgroundColor, padding);
    }

    TextTexture TextRenderer::RenderShapedToTexture(
        const Font &font,
        const ShapedText &shapedText,
        glm::u8vec4 textColor,
        glm::u8vec4 backgroundColor,
        uint32_t padding)
    {
        FT_Face face = font.GetFtFace();
        if (!face)
        {
            throw std::runtime_error("Cannot render text: Invalid FreeType face handle");
        }

        const uint32_t textWidth = static_cast<uint32_t>(shapedText.width);
        const uint32_t textHeight = static_cast<uint32_t>(shapedText.height);

        const uint32_t totalWidth = textWidth + padding * 2;
        const uint32_t totalHeight = textHeight + padding * 2;

        std::vector<uint8_t> pixels(totalWidth * totalHeight * 4, 0);

        // Fill background color
        for (size_t i = 0; i < totalWidth * totalHeight; ++i)
        {
            pixels[i * 4 + 0] = backgroundColor.r;
            pixels[i * 4 + 1] = backgroundColor.g;
            pixels[i * 4 + 2] = backgroundColor.b;
            pixels[i * 4 + 3] = backgroundColor.a;
        }

        const int32_t baselineY = static_cast<int32_t>(padding) + shapedText.maxAscent;
        int32_t currentX = static_cast<int32_t>(padding);

        for (const auto &glyph : shapedText.glyphs)
        {
            if (FT_Load_Glyph(face, glyph.codepoint, FT_LOAD_RENDER) != 0)
            {
                currentX += glyph.xAdvance;
                continue;
            }

            const FT_Bitmap &bitmap = face->glyph->bitmap;
            const int32_t glyphLeft = currentX + glyph.xOffset + face->glyph->bitmap_left;
            const int32_t glyphTop = baselineY - face->glyph->bitmap_top - glyph.yOffset;

            for (uint32_t row = 0; row < bitmap.rows; ++row)
            {
                const int32_t dstY = glyphTop + static_cast<int32_t>(row);
                if (dstY < 0 || dstY >= static_cast<int32_t>(totalHeight))
                {
                    continue;
                }

                for (uint32_t col = 0; col < bitmap.width; ++col)
                {
                    const int32_t dstX = glyphLeft + static_cast<int32_t>(col);
                    if (dstX < 0 || dstX >= static_cast<int32_t>(totalWidth))
                    {
                        continue;
                    }

                    const uint8_t srcAlpha = bitmap.buffer[row * bitmap.pitch + col];
                    if (srcAlpha == 0)
                    {
                        continue;
                    }

                    const size_t pixelIdx = (static_cast<size_t>(dstY) * totalWidth + static_cast<size_t>(dstX)) * 4;

                    const float fgFactor = (srcAlpha / 255.0f) * (textColor.a / 255.0f);
                    const float bgFactor = (pixels[pixelIdx + 3] / 255.0f) * (1.0f - fgFactor);
                    const float outAlpha = fgFactor + bgFactor;

                    if (outAlpha > 0.001f)
                    {
                        pixels[pixelIdx + 0] = static_cast<uint8_t>((textColor.r * fgFactor + pixels[pixelIdx + 0] * bgFactor) / outAlpha);
                        pixels[pixelIdx + 1] = static_cast<uint8_t>((textColor.g * fgFactor + pixels[pixelIdx + 1] * bgFactor) / outAlpha);
                        pixels[pixelIdx + 2] = static_cast<uint8_t>((textColor.b * fgFactor + pixels[pixelIdx + 2] * bgFactor) / outAlpha);
                        pixels[pixelIdx + 3] = static_cast<uint8_t>(std::min(255.0f, outAlpha * 255.0f));
                    }
                }
            }

            currentX += glyph.xAdvance;
        }

        // Upload CPU RGBA buffer to OpenGL 2D Texture using Direct State Access
        GLuint textureId = 0;
        glCreateTextures(GL_TEXTURE_2D, 1, &textureId);
        glTextureStorage2D(textureId, 1, GL_RGBA8, totalWidth, totalHeight);
        glTextureSubImage2D(textureId, 0, 0, 0, totalWidth, totalHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

        glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        spdlog::info("Generated OpenGL Text Texture ID {} ({}x{})", textureId, totalWidth, totalHeight);

        return TextTexture(textureId, totalWidth, totalHeight);
    }
} // namespace bgl
