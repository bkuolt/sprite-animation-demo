// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <hb-ft.h>
#include <hb.h>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace bgl
{
class FontLibrary
{
  public:
    static FontLibrary &Instance();

    FT_Library GetFtLibrary() const
    {
        return m_ftLibrary;
    }

    FontLibrary(const FontLibrary &) = delete;
    FontLibrary &operator=(const FontLibrary &) = delete;

  private:
    FontLibrary();
    ~FontLibrary();

    FT_Library m_ftLibrary{nullptr};
};

class Font
{
  public:
    Font(std::string_view fontPath, uint32_t pixelSize);
    Font(const uint8_t *data, size_t dataSize, uint32_t pixelSize);

    // Dynamically locate and load a system font using fontconfig
    static Font LoadSystemFont(std::string_view fontName, uint32_t pixelSize);

    ~Font();

    Font(const Font &) = delete;
    Font &operator=(const Font &) = delete;

    Font(Font &&other) noexcept;
    Font &operator=(Font &&other) noexcept;

    void SetPixelSize(uint32_t pixelSize);
    [[nodiscard]] uint32_t GetPixelSize() const
    {
        return m_pixelSize;
    }

    [[nodiscard]] FT_Face GetFtFace() const
    {
        return m_face;
    }
    [[nodiscard]] hb_font_t *GetHbFont() const
    {
        return m_hbFont;
    }

    [[nodiscard]] int32_t GetAscent() const;
    [[nodiscard]] int32_t GetDescent() const;
    [[nodiscard]] int32_t GetLineHeight() const;

  private:
    void Cleanup() noexcept;

    FT_Face m_face{nullptr};
    hb_font_t *m_hbFont{nullptr};
    std::vector<uint8_t> m_fontBuffer; // Keeps memory alive if loaded from memory
    uint32_t m_pixelSize{0};
};
} // namespace bgl
