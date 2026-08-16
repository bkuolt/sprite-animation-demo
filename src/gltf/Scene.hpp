// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "Node.hpp"
#include <memory>
#include <string>
#include <vector>

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

    void addRootNode(std::shared_ptr<Node> node);
    [[nodiscard]] const std::vector<std::shared_ptr<Node>> &getRootNodes() const noexcept;

    void keepTextureAlive(std::shared_ptr<class Texture2DArray> tex);

    void updateTransforms();
    [[nodiscard]] const std::string &getName() const noexcept;

  private:
    std::string _name;
    std::vector<std::shared_ptr<Node>> _rootNodes;
    std::vector<std::shared_ptr<class Texture2DArray>> _textures;
};
} // namespace bgl::gfx
