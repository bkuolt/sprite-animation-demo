// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "camera.hpp"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

namespace bgl::gfx
{

void Camera::handleScroll(double yoffset)
{
    if (yoffset > 0)
    {
        m_zoomLevel *= 0.9f;
    }
    else if (yoffset < 0)
    {
        m_zoomLevel *= 1.1f;
    }
    m_zoomLevel = std::clamp(m_zoomLevel, 0.1f, 10.0f);
    spdlog::info("Camera zoom: {:.2f}", m_zoomLevel);
}

void Camera::handleMouseButton(int button, int action)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_MIDDLE || button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
        {
            m_isPanning = true;
        }
        else if (action == GLFW_RELEASE)
        {
            m_isPanning = false;
        }
    }
}

void Camera::handleCursorPos(double xpos, double ypos, float windowWidth, float windowHeight)
{
    const glm::vec2 currentPos{static_cast<float>(xpos), static_cast<float>(ypos)};
    if (m_isPanning)
    {
        const glm::vec2 delta = currentPos - m_lastMousePos;
        const float aspect = windowWidth / windowHeight;

        const float worldWidth = 2.0f * m_zoomLevel * aspect;
        const float worldHeight = 2.0f * m_zoomLevel;

        m_cameraPosition.x -= delta.x * (worldWidth / windowWidth);
        m_cameraPosition.y += delta.y * (worldHeight / windowHeight);
    }
    m_lastMousePos = currentPos;
}

void Camera::reset()
{
    m_cameraPosition = {0.0f, 0.0f};
    m_zoomLevel = 1.0f;
    spdlog::info("Camera reset to position (0, 0) and zoom 1.0");
}

glm::mat4 Camera::getProjectionMatrix(float aspect) const
{
    return glm::ortho(m_cameraPosition.x - m_zoomLevel * aspect, m_cameraPosition.x + m_zoomLevel * aspect,
                      m_cameraPosition.y - m_zoomLevel, m_cameraPosition.y + m_zoomLevel, -1.0f, 1.0f);
}

} // namespace bgl::gfx
