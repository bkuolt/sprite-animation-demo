// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#pragma once

#include "Font.hpp"
#include "Graphics.hpp"
#include "Text_renderer.hpp"
#include "../Character.hpp"

#include <glm/vec2.hpp>
#include <memory>
#include <optional>
#include <string>

namespace bgl::gfx
{

class Hud
{
  public:
    Hud() = default;

    void updateAndRender(const Font &font, GLuint textProgram, const QuadMesh &overlayQuad, const glm::vec2 &winSize,
                         int currentFps, const std::shared_ptr<Character> &currentCharacter);

  private:
    std::string m_lastHudText1;
    std::string m_lastHudText2;
    std::optional<TextTexture> m_hudTexture1;
    std::optional<TextTexture> m_hudTexture2;
};

} // namespace bgl::gfx
