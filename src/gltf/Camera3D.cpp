// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "Camera3D.hpp"
#include <algorithm>

namespace bgl::gfx
{
Camera3D::Camera3D(glm::vec3 target, float distance)
    : _target(target), _distance(distance)
{
}



glm::mat4 Camera3D::getViewMatrix() const
{
    return glm::lookAt(getPosition(), _target, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera3D::getProjectionMatrix(float aspectRatio) const
{
    return glm::perspective(glm::radians(_fov), aspectRatio, 0.1f, 1000.0f);
}

glm::vec3 Camera3D::getPosition() const
{
    float yawRad = glm::radians(_yaw);
    float pitchRad = glm::radians(_pitch);

    float x = _target.x + _distance * std::cos(pitchRad) * std::sin(yawRad);
    float y = _target.y + _distance * std::sin(pitchRad);
    float z = _target.z + _distance * std::cos(pitchRad) * std::cos(yawRad);

    return glm::vec3(x, y, z);
}

void Camera3D::setTarget(const glm::vec3 &target) noexcept
{
    _target = target;
}

void Camera3D::setDistance(float distance) noexcept
{
    _distance = std::max(0.1f, distance);
}

void Camera3D::setPitch(float pitch) noexcept
{
    _pitch = std::clamp(pitch, -89.0f, 89.0f);
}

void Camera3D::setYaw(float yaw) noexcept
{
    _yaw = yaw;
}

void Camera3D::handleScroll(double yoffset)
{
    _distance -= static_cast<float>(yoffset) * 0.5f;
    _distance = std::max(0.1f, _distance);
}

void Camera3D::handleCursorPos(double xpos, double ypos)
{
    double dx = xpos - _lastMouseX;
    double dy = ypos - _lastMouseY;

    if (_isRotating)
    {
        _yaw += static_cast<float>(dx) * 0.25f;
        _pitch += static_cast<float>(dy) * 0.25f;
        _pitch = std::clamp(_pitch, -89.0f, 89.0f);
    }
    else if (_isPanning)
    {
        float panSpeed = _distance * 0.001f;
        glm::vec3 right = glm::normalize(glm::cross(getPosition() - _target, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        _target -= right * static_cast<float>(dx) * panSpeed;
        _target += up * static_cast<float>(dy) * panSpeed;
    }

    _lastMouseX = xpos;
    _lastMouseY = ypos;
}

void Camera3D::handleMouseButton(int button, int action)
{
    if (button == 0) // Left Mouse Button
    {
        _isRotating = (action == 1); // 1 = Press
    }
    else if (button == 1 || button == 2) // Right or Middle Mouse Button
    {
        _isPanning = (action == 1);
    }
}
} // namespace bgl::gfx
