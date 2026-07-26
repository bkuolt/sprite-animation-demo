// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

namespace bgl::gfx
{

class Camera
{
  public:
    Camera() = default;

    void handleScroll(double yoffset);
    void handleMouseButton(int button, int action);
    void handleCursorPos(double xpos, double ypos, float windowWidth, float windowHeight);
    void reset();

    [[nodiscard]] glm::mat4 getProjectionMatrix(float aspect) const;

  private:
    float m_zoomLevel = 1.0f;
    glm::vec2 m_cameraPosition{0.0f, 0.0f};
    bool m_isPanning = false;
    glm::vec2 m_lastMousePos{0.0f, 0.0f};
};

} // namespace bgl::gfx
