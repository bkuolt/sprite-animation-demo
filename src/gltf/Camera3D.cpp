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

void Camera3D::registerCallbacks(GLFWwindow *window)
{
    if (!window) return;
    glfwSetWindowUserPointer(window, this);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
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

void Camera3D::scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    (void)xoffset;
    auto *camera = static_cast<Camera3D *>(glfwGetWindowUserPointer(window));
    if (camera)
    {
        camera->_distance -= static_cast<float>(yoffset) * 0.5f;
        camera->_distance = std::max(0.1f, camera->_distance);
    }
}

void Camera3D::cursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    auto *camera = static_cast<Camera3D *>(glfwGetWindowUserPointer(window));
    if (!camera) return;

    double dx = xpos - camera->_lastMouseX;
    double dy = ypos - camera->_lastMouseY;

    if (camera->_isRotating)
    {
        camera->_yaw += static_cast<float>(dx) * 0.25f;
        camera->_pitch += static_cast<float>(dy) * 0.25f;
        camera->_pitch = std::clamp(camera->_pitch, -89.0f, 89.0f);
    }
    else if (camera->_isPanning)
    {
        float panSpeed = camera->_distance * 0.001f;
        glm::vec3 right = glm::normalize(glm::cross(camera->getPosition() - camera->_target, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        camera->_target -= right * static_cast<float>(dx) * panSpeed;
        camera->_target += up * static_cast<float>(dy) * panSpeed;
    }

    camera->_lastMouseX = xpos;
    camera->_lastMouseY = ypos;
}

void Camera3D::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    (void)mods;
    auto *camera = static_cast<Camera3D *>(glfwGetWindowUserPointer(window));
    if (!camera) return;

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        camera->_isRotating = (action == GLFW_PRESS);
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT || button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        camera->_isPanning = (action == GLFW_PRESS);
    }
}
} // namespace bgl::gfx
