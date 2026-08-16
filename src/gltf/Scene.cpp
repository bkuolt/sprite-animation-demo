// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "Scene.hpp"
#include <utility>

namespace bgl::gfx
{
Scene::Scene(std::string name)
    : _name(std::move(name))
{
}

void Scene::addRootNode(std::shared_ptr<Node> node)
{
    _rootNodes.push_back(std::move(node));
}

void Scene::keepTextureAlive(std::shared_ptr<bgl::gl::Texture2DArray> tex)
{
    if (tex) {
        m_textures.push_back(std::move(tex));
    }
}

const std::vector<std::shared_ptr<Node>> &Scene::getRootNodes() const noexcept
{
    return _rootNodes;
}

void Scene::updateTransforms(const glm::mat4 &rootMatrix)
{
    for (auto &rootNode : _rootNodes)
    {
        if (rootNode)
        {
            rootNode->updateTransforms(rootMatrix);
        }
    }
}

const std::string &Scene::getName() const noexcept
{
    return _name;
}
} // namespace bgl::gfx
