// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#include "textShaper.hpp"

#include <algorithm>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace bgl
{
ShapedText TextShaper::ShapeText(const Font &font, std::string_view text, hb_direction_t direction, hb_script_t script,
                                 const char *language)
{
    if (text.empty())
    {
        return {};
    }

    hb_font_t *hbFont = font.GetHbFont();
    if (!hbFont)
    {
        throw std::runtime_error("Cannot shape text: Font object does not contain a valid HarfBuzz handle");
    }

    hb_buffer_t *buffer = hb_buffer_create();
    if (!buffer)
    {
        throw std::runtime_error("Failed to create HarfBuzz buffer");
    }

    hb_buffer_add_utf8(buffer, text.data(), static_cast<int>(text.size()), 0, static_cast<int>(text.size()));
    hb_buffer_set_direction(buffer, direction);
    hb_buffer_set_script(buffer, script);
    if (language && *language != '\0')
    {
        hb_buffer_set_language(buffer, hb_language_from_string(language, -1));
    }

    // Perform HarfBuzz text shaping
    hb_shape(hbFont, buffer, nullptr, 0);

    unsigned int glyphCount = 0;
    hb_glyph_info_t *glyphInfos = hb_buffer_get_glyph_infos(buffer, &glyphCount);
    hb_glyph_position_t *glyphPositions = hb_buffer_get_glyph_positions(buffer, &glyphCount);

    ShapedText shapedText;
    shapedText.glyphs.reserve(glyphCount);
    shapedText.maxAscent = font.GetAscent();
    shapedText.maxDescent = font.GetDescent();

    int32_t currentX = 0;
    int32_t currentY = 0;

    FT_Face face = font.GetFtFace();

    for (unsigned int i = 0; i < glyphCount; ++i)
    {
        ShapedGlyph glyph{};
        glyph.codepoint = glyphInfos[i].codepoint;
        glyph.cluster = glyphInfos[i].cluster;

        // HarfBuzz 26.6 fractional units -> pixels
        glyph.xAdvance = glyphPositions[i].x_advance >> 6;
        glyph.yAdvance = glyphPositions[i].y_advance >> 6;
        glyph.xOffset = glyphPositions[i].x_offset >> 6;
        glyph.yOffset = glyphPositions[i].y_offset >> 6;

        // Compute exact metrics for overall bounding box calculation
        if (face && FT_Load_Glyph(face, glyph.codepoint, FT_LOAD_DEFAULT) == 0)
        {
            const FT_Glyph_Metrics &metrics = face->glyph->metrics;
            int32_t bearingY = metrics.horiBearingY >> 6;
            int32_t glyphHeight = metrics.height >> 6;
            int32_t descent = glyphHeight - bearingY;

            shapedText.maxAscent = std::max(shapedText.maxAscent, bearingY);
            shapedText.maxDescent = std::max(shapedText.maxDescent, descent);
        }

        shapedText.glyphs.push_back(glyph);
        currentX += glyph.xAdvance;
        currentY += glyph.yAdvance;
    }

    shapedText.totalAdvanceX = currentX;
    shapedText.totalAdvanceY = currentY;
    shapedText.width = std::max(1, currentX);
    shapedText.height = std::max(1, shapedText.maxAscent + shapedText.maxDescent);

    hb_buffer_destroy(buffer);
    return shapedText;
}
} // namespace bgl
