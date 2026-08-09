// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "Node.hpp"
#include <glm/gtx/quaternion.hpp>
#include <utility>

namespace bgl::gfx
{
Node::Node(std::string name)
    : _name(std::move(name))
{
}

void Node::setTranslation(const glm::vec3 &translation) noexcept
{
    _translation = translation;
    _hasCustomMatrix = false;
    _isMatrixDirty = true;
}

void Node::setRotation(const glm::quat &rotation) noexcept
{
    _rotation = rotation;
    _hasCustomMatrix = false;
    _isMatrixDirty = true;
}

void Node::setScale(const glm::vec3 &scale) noexcept
{
    _scale = scale;
    _hasCustomMatrix = false;
    _isMatrixDirty = true;
}

void Node::setLocalMatrix(const glm::mat4 &matrix) noexcept
{
    _localMatrix = matrix;
    _hasCustomMatrix = true;
    _isMatrixDirty = false;
}

const glm::vec3 &Node::getTranslation() const noexcept
{
    return _translation;
}

const glm::quat &Node::getRotation() const noexcept
{
    return _rotation;
}

const glm::vec3 &Node::getScale() const noexcept
{
    return _scale;
}

glm::mat4 Node::getLocalMatrix() const noexcept
{
    if (_hasCustomMatrix)
    {
        return _localMatrix;
    }

    glm::mat4 t = glm::translate(glm::mat4(1.0f), _translation);
    glm::mat4 r = glm::mat4_cast(_rotation);
    glm::mat4 s = glm::scale(glm::mat4(1.0f), _scale);
    return t * r * s;
}

void Node::addChild(std::shared_ptr<Node> child)
{
    if (child)
    {
        child->_parent = shared_from_this();
        _children.push_back(std::move(child));
    }
}

const std::vector<std::shared_ptr<Node>> &Node::getChildren() const noexcept
{
    return _children;
}

std::shared_ptr<Node> Node::getParent() const noexcept
{
    return _parent.lock();
}

void Node::updateTransforms(const glm::mat4 &parentTransform)
{
    glm::mat4 local = getLocalMatrix();
    _globalMatrix = parentTransform * local;

    for (auto &child : _children)
    {
        if (child)
        {
            child->updateTransforms(_globalMatrix);
        }
    }
}

const glm::mat4 &Node::getGlobalMatrix() const noexcept
{
    return _globalMatrix;
}

void Node::addMesh(std::shared_ptr<Mesh> mesh)
{
    if (mesh)
    {
        _meshes.push_back(std::move(mesh));
    }
}

const std::vector<std::shared_ptr<Mesh>> &Node::getMeshes() const noexcept
{
    return _meshes;
}

const std::string &Node::getName() const noexcept
{
    return _name;
}
} // namespace bgl::gfx
