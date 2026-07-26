// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#include "Hud.hpp"
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
        const std::string currentHudText1 = fmt::format("{} FPS", currentFps);
        const std::string currentHudText2 =
            fmt::format("Char: {}, File: {}, Anim: {}, Frames {}", currentCharacter->getName(), currentFilename,
                        animState->name, animState->frameCount);

        if (currentHudText1 != m_lastHudText1 || !m_hudTexture1.has_value())
        {
            m_lastHudText1 = currentHudText1;
            m_hudTexture1 = TextRenderer::RenderToTexture(font, currentHudText1, {0, 100, 255, 255}, // Blue text color
                                                          {0, 0, 0, 0},                              // Transparent background
                                                          4                                          // Padding
            );
        }

        if (currentHudText2 != m_lastHudText2 || !m_hudTexture2.has_value())
        {
            m_lastHudText2 = currentHudText2;
            m_hudTexture2 = TextRenderer::RenderToTexture(font, currentHudText2, {0, 100, 255, 255}, // Blue text color
                                                          {0, 0, 0, 0},                              // Transparent background
                                                          4                                          // Padding
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
