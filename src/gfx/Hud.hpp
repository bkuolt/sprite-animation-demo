// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "Graphics.hpp"
#include "text/Font.hpp"
#include "text/TextRenderer.hpp"

#include <glm/vec2.hpp>
#include <optional>
#include <string>

namespace bgl { class Character; struct AnimationState; }

namespace bgl::gfx
{

class Hud
{
  public:
    Hud() = default;

    /// Renders the HUD overlay with FPS and animation info.
    /// @param modelName The name of the currently rendered 3D model.
    void updateAndRender(const Font &font, GLuint textProgram, const QuadMesh &overlayQuad,
                         const glm::vec2 &winSize, int currentFps, const std::string &modelName);

  private:
    std::string m_lastHudText1;
    std::string m_lastHudText2;
    std::optional<TextTexture> m_hudTexture1;
    std::optional<TextTexture> m_hudTexture2;
    std::optional<TextTexture> m_hudTexture3;
};

} // namespace bgl::gfx
