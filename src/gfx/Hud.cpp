// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "Hud.hpp"
#include "../Character.hpp"
#include <fmt/format.h>

namespace bgl::gfx
{

void Hud::updateAndRender(const Font &font, GLuint textProgram, const QuadMesh &overlayQuad, const glm::vec2 &winSize,
                          int currentFps, const std::string &modelName)
{
    const std::string fullText = fmt::format("{} FPS \n Model: {}", currentFps, modelName);

    if (fullText != m_lastHudText1 || !m_hudTexture1.has_value() || !m_hudTexture2.has_value() || !m_hudTexture3.has_value())
    {
        m_lastHudText1 = fullText;

        // Split the fullText into two lines by \n
        size_t newlinePos = fullText.find('\n');
        std::string line1 = fullText.substr(0, newlinePos);
        std::string line2 = fullText.substr(newlinePos + 1);
        std::string line3 = "QML Overlay: [Overlay.qml] | GPU Culling: ACTIVE";

        m_hudTexture1 = TextRenderer::RenderToTexture(font, line1, {0, 230, 118, 255}, // Bright neon green FPS
                                                      {15, 23, 42, 200},               // Dark translucent background
                                                      6                                // Padding
        );

        m_hudTexture2 = TextRenderer::RenderToTexture(font, line2, {100, 181, 246, 255}, // Light blue text color
                                                      {15, 23, 42, 200},                  // Dark translucent background
                                                      6                                   // Padding
        );

        m_hudTexture3 = TextRenderer::RenderToTexture(font, line3, {255, 183, 77, 255}, // Amber/gold QML badge
                                                      {15, 23, 42, 220},                 // Dark translucent background
                                                      6                                  // Padding
        );
    }

    float currentY = 15.0f;
    if (m_hudTexture1 && m_hudTexture1->IsValid())
    {
        renderTextOverlay(overlayQuad, m_hudTexture1->GetHandle(), textProgram, m_hudTexture1->GetWidth(),
                          m_hudTexture1->GetHeight(), static_cast<uint32_t>(winSize.x),
                          static_cast<uint32_t>(winSize.y), 15.0f, currentY);
        currentY += m_hudTexture1->GetHeight() + 6.0f;
    }

    if (m_hudTexture2 && m_hudTexture2->IsValid())
    {
        renderTextOverlay(overlayQuad, m_hudTexture2->GetHandle(), textProgram, m_hudTexture2->GetWidth(),
                          m_hudTexture2->GetHeight(), static_cast<uint32_t>(winSize.x),
                          static_cast<uint32_t>(winSize.y), 15.0f, currentY);
        currentY += m_hudTexture2->GetHeight() + 6.0f;
    }

    if (m_hudTexture3 && m_hudTexture3->IsValid())
    {
        renderTextOverlay(overlayQuad, m_hudTexture3->GetHandle(), textProgram, m_hudTexture3->GetWidth(),
                          m_hudTexture3->GetHeight(), static_cast<uint32_t>(winSize.x),
                          static_cast<uint32_t>(winSize.y), 15.0f, currentY);
    }
}

} // namespace bgl::gfx
