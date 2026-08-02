// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "textureAtlas.hpp"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#include <algorithm>
#include <cmath>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <utility>

namespace bgl
{

TextureAtlas::TextureAtlas(GLuint textureId, uint32_t width, uint32_t height,
                           std::unordered_map<uint32_t, GlyphAtlasInfo> glyphs)
    : m_textureId(textureId), m_width(width), m_height(height), m_glyphs(std::move(glyphs))
{
}

TextureAtlas::~TextureAtlas()
{
    Cleanup();
}

TextureAtlas::TextureAtlas(TextureAtlas &&other) noexcept
    : m_textureId(other.m_textureId), m_width(other.m_width), m_height(other.m_height),
      m_glyphs(std::move(other.m_glyphs))
{
    other.m_textureId = 0;
    other.m_width = 0;
    other.m_height = 0;
}

TextureAtlas &TextureAtlas::operator=(TextureAtlas &&other) noexcept
{
    if (this != &other)
    {
        Cleanup();

        m_textureId = other.m_textureId;
        m_width = other.m_width;
        m_height = other.m_height;
        m_glyphs = std::move(other.m_glyphs);

        other.m_textureId = 0;
        other.m_width = 0;
        other.m_height = 0;
    }
    return *this;
}

void TextureAtlas::Cleanup() noexcept
{
    if (m_textureId != 0)
    {
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
        m_width = 0;
        m_height = 0;
        m_glyphs.clear();
    }
}

const GlyphAtlasInfo *TextureAtlas::GetGlyph(uint32_t codepoint) const
{
    auto it = m_glyphs.find(codepoint);
    if (it != m_glyphs.end())
    {
        return &it->second;
    }
    return nullptr;
}

void TextureAtlas::Bind(uint32_t slot) const
{
    if (m_textureId != 0)
    {
        glBindTextureUnit(slot, m_textureId);
    }
}

TextureAtlas GenerateTextureAtlas(const Font &font, uint32_t firstChar, uint32_t lastChar)
{
    FT_Face face = font.GetFtFace();
    if (!face)
    {
        throw std::runtime_error("Cannot generate texture atlas: Invalid FreeType face");
    }

    struct GlyphBitmapData
    {
        uint32_t charCode;
        uint32_t glyphIndex;
        int width;
        int height;
        int bearingX;
        int bearingY;
        int advanceX;
        std::vector<uint8_t> bitmapBuffer;
    };

    std::vector<GlyphBitmapData> glyphData;
    std::vector<stbrp_rect> stbRects;

    for (uint32_t c = firstChar; c <= lastChar; ++c)
    {
        FT_UInt glyphIndex = FT_Get_Char_Index(face, c);
        if (glyphIndex == 0 && c != ' ')
        {
            continue;
        }

        if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL) != 0)
        {
            spdlog::warn("Failed to load glyph for char: {}", c);
            continue;
        }

        const FT_Bitmap &bitmap = face->glyph->bitmap;

        GlyphBitmapData gd{};
        gd.charCode = c;
        gd.glyphIndex = glyphIndex;
        gd.width = static_cast<int>(bitmap.width);
        gd.height = static_cast<int>(bitmap.rows);
        gd.bearingX = face->glyph->bitmap_left;
        gd.bearingY = face->glyph->bitmap_top;
        gd.advanceX = face->glyph->advance.x >> 6;

        if (bitmap.width > 0 && bitmap.rows > 0)
        {
            gd.bitmapBuffer.assign(bitmap.buffer, bitmap.buffer + (bitmap.width * bitmap.rows));
        }

        stbrp_rect rect{};
        rect.id = static_cast<int>(glyphData.size());
        // 1px padding on each border to prevent filtering bleed
        rect.w = static_cast<stbrp_coord>(gd.width + 2);
        rect.h = static_cast<stbrp_coord>(gd.height + 2);

        stbRects.push_back(rect);
        glyphData.push_back(std::move(gd));
    }

    int atlasWidth = 512;
    int atlasHeight = 512;
    bool packedSuccessfully = false;

    while (atlasWidth <= 2048 && atlasHeight <= 2048)
    {
        std::vector<stbrp_node> nodes(atlasWidth);
        stbrp_context packContext{};
        stbrp_init_target(&packContext, atlasWidth, atlasHeight, nodes.data(), static_cast<int>(nodes.size()));

        if (stbrp_pack_rects(&packContext, stbRects.data(), static_cast<int>(stbRects.size())) != 0)
        {
            packedSuccessfully = true;
            break;
        }

        atlasWidth *= 2;
        atlasHeight *= 2;
    }

    if (!packedSuccessfully)
    {
        throw std::runtime_error("Failed to pack font glyphs into texture atlas");
    }

    std::vector<uint8_t> atlasBuffer(atlasWidth * atlasHeight, 0);
    std::unordered_map<uint32_t, GlyphAtlasInfo> glyphMap;

    for (const auto &rect : stbRects)
    {
        if (!rect.was_packed)
            continue;

        const auto &gd = glyphData[rect.id];
        int dstX = rect.x + 1;
        int dstY = rect.y + 1;

        for (int r = 0; r < gd.height; ++r)
        {
            for (int c = 0; c < gd.width; ++c)
            {
                atlasBuffer[(dstY + r) * atlasWidth + (dstX + c)] = gd.bitmapBuffer[r * gd.width + c];
            }
        }

        GlyphAtlasInfo info{};
        info.codepoint = gd.glyphIndex;
        info.u0 = static_cast<float>(dstX) / static_cast<float>(atlasWidth);
        info.v0 = static_cast<float>(dstY) / static_cast<float>(atlasHeight);
        info.u1 = static_cast<float>(dstX + gd.width) / static_cast<float>(atlasWidth);
        info.v1 = static_cast<float>(dstY + gd.height) / static_cast<float>(atlasHeight);
        info.width = gd.width;
        info.height = gd.height;
        info.bearingX = gd.bearingX;
        info.bearingY = gd.bearingY;
        info.advanceX = gd.advanceX;

        glyphMap[gd.charCode] = info;
        glyphMap[gd.glyphIndex] = info; // map by both charCode and FreeType glyph index
    }

    GLuint textureId = 0;
    glCreateTextures(GL_TEXTURE_2D, 1, &textureId);
    GLsizei mipLevels = static_cast<GLsizei>(1 + std::floor(std::log2(std::max(atlasWidth, atlasHeight))));
    glTextureStorage2D(textureId, mipLevels, GL_R8, atlasWidth, atlasHeight);
    glTextureSubImage2D(textureId, 0, 0, 0, atlasWidth, atlasHeight, GL_RED, GL_UNSIGNED_BYTE, atlasBuffer.data());

    glGenerateTextureMipmap(textureId);

    glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_RED};
    glTextureParameteriv(textureId, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

    spdlog::info("Generated Font Texture Atlas ID {} ({}x{}, {} glyphs packed, {} mip levels)", textureId, atlasWidth,
                 atlasHeight, glyphMap.size(), mipLevels);

    return TextureAtlas(textureId, static_cast<uint32_t>(atlasWidth), static_cast<uint32_t>(atlasHeight),
                        std::move(glyphMap));
}

} // namespace bgl
