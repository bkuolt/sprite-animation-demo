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

    void updateTransforms();
    [[nodiscard]] const std::string &getName() const noexcept;

  private:
    std::string _name;
    std::vector<std::shared_ptr<Node>> _rootNodes;
};
} // namespace bgl::gfx
