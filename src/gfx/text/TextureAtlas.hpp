// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "Font.hpp"

#include <glad/gl.h>
#include <glm/vec4.hpp>

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace bgl
{

struct GlyphAtlasInfo
{
    uint32_t codepoint{0};
    float u0{0.0f}, v0{0.0f}, u1{0.0f}, v1{0.0f}; // UV coordinates in texture atlas
    int32_t width{0};
    int32_t height{0};
    int32_t bearingX{0};
    int32_t bearingY{0};
    int32_t advanceX{0};
};

class TextureAtlas
{
  public:
    TextureAtlas() = default;
    TextureAtlas(GLuint textureId, uint32_t width, uint32_t height,
                 std::unordered_map<uint32_t, GlyphAtlasInfo> glyphs);
    ~TextureAtlas();

    TextureAtlas(const TextureAtlas &) = delete;
    TextureAtlas &operator=(const TextureAtlas &) = delete;

    TextureAtlas(TextureAtlas &&other) noexcept;
    TextureAtlas &operator=(TextureAtlas &&other) noexcept;

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
    [[nodiscard]] const GlyphAtlasInfo *GetGlyph(uint32_t codepoint) const;

    void Bind(uint32_t slot = 0) const;

  private:
    void Cleanup() noexcept;

    GLuint m_textureId{0};
    uint32_t m_width{0};
    uint32_t m_height{0};
    std::unordered_map<uint32_t, GlyphAtlasInfo> m_glyphs;
};

// Generates a 2D Texture Atlas for printable font characters using rectpack
[[nodiscard]] TextureAtlas GenerateTextureAtlas(const Font &font, uint32_t firstChar = 32, uint32_t lastChar = 126);

} // namespace bgl
