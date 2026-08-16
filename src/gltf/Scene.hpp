// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "Node.hpp"
#include <memory>
#include <string>
#include <vector>

namespace bgl::gl { class Texture2DArray; }

namespace bgl::gfx
{
/**
 * @brief Root container for the glTF scene graph.
 */
class Scene
{
  public:
    explicit Scene(std::string name = "");
    ~Scene() = default;

    /**
     * @brief Adds a root node to the scene graph.
     * @param node The node to add to the root of the hierarchy.
     */
    void addRootNode(std::shared_ptr<Node> node);

    /**
     * @brief Retrieves all root nodes in the scene.
     * @return A vector containing shared pointers to the root nodes.
     */
    [[nodiscard]] const std::vector<std::shared_ptr<Node>> &getRootNodes() const noexcept;

    /**
     * @brief Registers a texture to ensure its OpenGL handle stays alive for the scene's lifetime.
     * @param tex The texture array to keep alive.
     */
    void keepTextureAlive(std::shared_ptr<bgl::gl::Texture2DArray> tex);

    /**
     * @brief Recursively updates the global transform matrices for all nodes in the scene graph.
     * @param rootMatrix An optional root transform matrix to apply to the entire scene (e.g. for scaling).
     */
    void updateTransforms(const glm::mat4 &rootMatrix = glm::mat4(1.0f));

    /**
     * @brief Retrieves the scene's name.
     * @return The scene's name.
     */
    [[nodiscard]] const std::string &getName() const noexcept;

  private:
    std::string _name;
    std::vector<std::shared_ptr<Node>> _rootNodes;
    std::vector<std::shared_ptr<bgl::gl::Texture2DArray>> m_textures;
};
} // namespace bgl::gfx
