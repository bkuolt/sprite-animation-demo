// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#include "font.hpp"

#include <spdlog/spdlog.h>
#include <stdexcept>
#include <utility>

namespace bgl
{
    FontLibrary &FontLibrary::Instance()
    {
        static FontLibrary instance;
        return instance;
    }

    FontLibrary::FontLibrary()
    {
        if (FT_Init_FreeType(&m_ftLibrary) != 0)
        {
            throw std::runtime_error("Failed to initialize FreeType library");
        }
        spdlog::info("FreeType library initialized successfully");
    }

    FontLibrary::~FontLibrary()
    {
        if (m_ftLibrary)
        {
            FT_Done_FreeType(m_ftLibrary);
            m_ftLibrary = nullptr;
        }
    }

    Font::Font(std::string_view fontPath, uint32_t pixelSize)
    {
        FT_Library ft = FontLibrary::Instance().GetFtLibrary();

        std::string pathStr(fontPath);
        if (FT_New_Face(ft, pathStr.c_str(), 0, &m_face) != 0)
        {
            throw std::runtime_error(fmt::format("Failed to load font from path: {}", fontPath));
        }

        SetPixelSize(pixelSize);
        spdlog::info("Successfully loaded font: {} at {}px", fontPath, pixelSize);
    }

    Font::Font(const uint8_t *data, size_t dataSize, uint32_t pixelSize)
    {
        if (!data || dataSize == 0)
        {
            throw std::runtime_error("Invalid memory font buffer provided");
        }

        m_fontBuffer.assign(data, data + dataSize);
        FT_Library ft = FontLibrary::Instance().GetFtLibrary();

        if (FT_New_Memory_Face(ft, m_fontBuffer.data(), static_cast<FT_Long>(m_fontBuffer.size()), 0, &m_face) != 0)
        {
            throw std::runtime_error("Failed to load font from memory buffer");
        }

        SetPixelSize(pixelSize);
        spdlog::info("Successfully loaded memory font at {}px", pixelSize);
    }

    Font::~Font()
    {
        Cleanup();
    }

    Font::Font(Font &&other) noexcept
        : m_face(other.m_face),
          m_hbFont(other.m_hbFont),
          m_fontBuffer(std::move(other.m_fontBuffer)),
          m_pixelSize(other.m_pixelSize)
    {
        other.m_face = nullptr;
        other.m_hbFont = nullptr;
        other.m_pixelSize = 0;
    }

    Font &Font::operator=(Font &&other) noexcept
    {
        if (this != &other)
        {
            Cleanup();

            m_face = other.m_face;
            m_hbFont = other.m_hbFont;
            m_fontBuffer = std::move(other.m_fontBuffer);
            m_pixelSize = other.m_pixelSize;

            other.m_face = nullptr;
            other.m_hbFont = nullptr;
            other.m_pixelSize = 0;
        }
        return *this;
    }

    void Font::Cleanup() noexcept
    {
        if (m_hbFont)
        {
            hb_font_destroy(m_hbFont);
            m_hbFont = nullptr;
        }
        if (m_face)
        {
            FT_Done_Face(m_face);
            m_face = nullptr;
        }
    }

    void Font::SetPixelSize(uint32_t pixelSize)
    {
        m_pixelSize = pixelSize;

        if (m_face)
        {
            if (FT_Set_Pixel_Sizes(m_face, 0, pixelSize) != 0)
            {
                spdlog::warn("Failed to set font pixel size to {}", pixelSize);
            }
        }

        if (m_hbFont)
        {
            hb_font_destroy(m_hbFont);
            m_hbFont = nullptr;
        }

        if (m_face)
        {
            m_hbFont = hb_ft_font_create(m_face, nullptr);
            if (!m_hbFont)
            {
                throw std::runtime_error("Failed to create HarfBuzz font wrapper from FreeType face");
            }
            hb_ft_font_set_funcs(m_hbFont);
        }
    }

    int32_t Font::GetAscent() const
    {
        return m_face ? (m_face->size->metrics.ascender >> 6) : 0;
    }

    int32_t Font::GetDescent() const
    {
        return m_face ? (m_face->size->metrics.descender >> 6) : 0;
    }

    int32_t Font::GetLineHeight() const
    {
        return m_face ? (m_face->size->metrics.height >> 6) : 0;
    }
} // namespace bgl
