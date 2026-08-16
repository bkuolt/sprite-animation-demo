// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace bgl::gfx
{
/**
 * @brief Orbit 3D Camera designed for easy panning, rotating, and zooming.
 * Integrates with the application event system for input handling.
 */
class Camera3D
{
  public:
    explicit Camera3D(glm::vec3 target = glm::vec3(0.0f), float distance = 5.0f);
    ~Camera3D() = default;

    /**
     * @brief Handles mouse scroll events for zooming.
     * @param yoffset The scroll offset.
     */
    void handleScroll(double yoffset);

    /**
     * @brief Handles cursor position updates for rotating and panning.
     * @param xpos New cursor X position.
     * @param ypos New cursor Y position.
     */
    void handleCursorPos(double xpos, double ypos);

    /**
     * @brief Handles mouse button states to toggle rotation or panning modes.
     * @param button The mouse button pressed or released.
     * @param action The action (e.g. GLFW_PRESS or GLFW_RELEASE).
     */
    void handleMouseButton(int button, int action);

    /**
     * @brief Generates the View matrix based on the current orbit parameters.
     * @return The 4x4 View matrix.
     */
    [[nodiscard]] glm::mat4 getViewMatrix() const;

    /**
     * @brief Generates the Projection matrix.
     * @param aspectRatio The aspect ratio of the viewport.
     * @return The 4x4 Projection matrix.
     */
    [[nodiscard]] glm::mat4 getProjectionMatrix(float aspectRatio) const;

    /**
     * @brief Calculates the world-space position of the camera.
     * @return The 3D position vector.
     */
    [[nodiscard]] glm::vec3 getPosition() const;

    /**
     * @brief Sets the orbit target (the center point the camera looks at).
     * @param target The new target position.
     */
    void setTarget(const glm::vec3 &target) noexcept;

    /**
     * @brief Sets the orbit distance from the target.
     * @param distance The new distance.
     */
    void setDistance(float distance) noexcept;
    void setPitch(float pitch) noexcept;
    void setYaw(float yaw) noexcept;

  private:


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
