// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "font.hpp"

#include <hb.h>

#include <cstdint>
#include <string_view>
#include <vector>

namespace bgl
{
struct ShapedGlyph
{
    uint32_t codepoint{0}; // Glyph index in font face
    int32_t xAdvance{0};   // Position advance (pixels)
    int32_t yAdvance{0};
    int32_t xOffset{0}; // Glyph render offset (pixels)
    int32_t yOffset{0};
    uint32_t cluster{0}; // Source character cluster index
};

struct ShapedText
{
    std::vector<ShapedGlyph> glyphs;
    int32_t totalAdvanceX{0};
    int32_t totalAdvanceY{0};
    int32_t width{0};
    int32_t height{0};
    int32_t maxAscent{0};
    int32_t maxDescent{0};
};

class TextShaper
{
  public:
    [[nodiscard]] static ShapedText ShapeText(const Font &font, std::string_view text,
                                              hb_direction_t direction = HB_DIRECTION_LTR,
                                              hb_script_t script = HB_SCRIPT_LATIN, const char *language = "en");
};
} // namespace bgl
