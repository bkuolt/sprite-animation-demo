// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "hud.hpp"
#include <fmt/format.h>

namespace bgl::gfx
{

void Hud::updateAndRender(const Font &font, GLuint textProgram, const QuadMesh &overlayQuad, const glm::vec2 &winSize,
                          int currentFps, const std::shared_ptr<Character> &currentCharacter)
{
    const auto *animState = currentCharacter ? currentCharacter->getCurrentAnimation() : nullptr;

    if (animState)
    {
        const std::string currentFilename = fmt::format("{}.ktx2", animState->name);
        // EXACT requested format string: %i FPS \n File: %s, Animation: %s, Frames %i
        const std::string fullText = fmt::format("{} FPS \n File: {}, Animation: {}, Frames {}", currentFps,
                                                 currentFilename, animState->name, animState->frameCount);

        if (fullText != m_lastHudText1 || !m_hudTexture1.has_value() || !m_hudTexture2.has_value())
        {
            m_lastHudText1 = fullText;

            // Split the fullText into two lines by \n
            size_t newlinePos = fullText.find('\n');
            std::string line1 = fullText.substr(0, newlinePos);
            std::string line2 = fullText.substr(newlinePos + 1);

            m_hudTexture1 = TextRenderer::RenderToTexture(font, line1, {0, 100, 255, 255}, // Blue text color
                                                          {0, 0, 0, 0},                    // Transparent background
                                                          4                                // Padding
            );

            m_hudTexture2 = TextRenderer::RenderToTexture(font, line2, {0, 100, 255, 255}, // Blue text color
                                                          {0, 0, 0, 0},                    // Transparent background
                                                          4                                // Padding
            );
        }
    }

    if (m_hudTexture1 && m_hudTexture1->IsValid())
    {
        renderTextOverlay(overlayQuad, m_hudTexture1->GetHandle(), textProgram, m_hudTexture1->GetWidth(),
                          m_hudTexture1->GetHeight(), static_cast<uint32_t>(winSize.x),
                          static_cast<uint32_t>(winSize.y), 15.0f, 15.0f);
    }

    if (m_hudTexture2 && m_hudTexture2->IsValid() && m_hudTexture1 && m_hudTexture1->IsValid())
    {
        renderTextOverlay(overlayQuad, m_hudTexture2->GetHandle(), textProgram, m_hudTexture2->GetWidth(),
                          m_hudTexture2->GetHeight(), static_cast<uint32_t>(winSize.x),
                          static_cast<uint32_t>(winSize.y), 15.0f, 15.0f + m_hudTexture1->GetHeight() + 5.0f);
    }
}

} // namespace bgl::gfx
