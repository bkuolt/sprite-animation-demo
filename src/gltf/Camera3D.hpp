// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace bgl::gfx
{
/**
 * @brief Orbit 3D Camera using GLFW callbacks for interaction.
 */
class Camera3D
{
  public:
    explicit Camera3D(glm::vec3 target = glm::vec3(0.0f), float distance = 5.0f);
    ~Camera3D() = default;

    void registerCallbacks(GLFWwindow *window);

    [[nodiscard]] glm::mat4 getViewMatrix() const;
    [[nodiscard]] glm::mat4 getProjectionMatrix(float aspectRatio) const;
    [[nodiscard]] glm::vec3 getPosition() const;

    void setTarget(const glm::vec3 &target) noexcept;
    void setDistance(float distance) noexcept;

  private:
    static void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    static void cursorPosCallback(GLFWwindow *window, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

    glm::vec3 _target{0.0f};
    float _distance{5.0f};
    float _yaw{0.0f};
    float _pitch{20.0f};
    float _fov{45.0f};

    bool _isRotating{false};
    bool _isPanning{false};
    double _lastMouseX{0.0};
    double _lastMouseY{0.0};
};
} // namespace bgl::gfx
