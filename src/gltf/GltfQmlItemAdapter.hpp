// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "GltfRenderer.hpp"
#include "Scene.hpp"
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace bgl::gfx
{
/**
 * @brief Window-Agnostic Qt/QML Bridge Item for glTF 3D rendering.
 * 
 * Provides a clean adapter for integrating bgl::gfx::GltfRenderer
 * into Qt QML UI pipelines (e.g. QQuickFramebufferObject or QtQuick 3D custom items).
 */
class GltfQmlItemAdapter
{
  public:
    GltfQmlItemAdapter()
        : _renderer(std::make_unique<GltfRenderer>())
    {
    }

    ~GltfQmlItemAdapter() = default;

    /**
     * @brief Set the glTF scene to be rendered inside the QML viewport.
     */
    void setScene(std::shared_ptr<Scene> scene)
    {
        _scene = std::move(scene);
    }

    /**
     * @brief Update method to be invoked inside QML's render/update cycle.
     */
    void update(float deltaTime)
    {
        if (_renderer && _scene)
        {
            _renderer->update(_scene, deltaTime);
        }
    }

    /**
     * @brief Render method to be called inside QML FBO / OpenGL context.
     */
    void render(int viewportWidth, int viewportHeight, const glm::vec3 &cameraPos, const glm::vec3 &targetPos)
    {
        if (!_renderer || !_scene || viewportWidth <= 0 || viewportHeight <= 0)
        {
            return;
        }

        float aspect = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);
        glm::mat4 view = glm::lookAt(cameraPos, targetPos, glm::vec3(0.0f, 1.0f, 0.0f));

        _renderer->render(_scene, view, projection, cameraPos);
    }

    [[nodiscard]] GltfRenderer *getRenderer() noexcept
    {
        return _renderer.get();
    }

  private:
    std::unique_ptr<GltfRenderer> _renderer;
    std::shared_ptr<Scene> _scene;
};
} // namespace bgl::gfx
