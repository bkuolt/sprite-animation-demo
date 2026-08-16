// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "Mesh.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <string>
#include <vector>

namespace bgl::gfx
{
/**
 * @brief Node in the hierarchical scene graph.
 */
class Node : public std::enable_shared_from_this<Node>
{
  public:
    explicit Node(std::string name = "");
    ~Node() = default;

    void setTranslation(const glm::vec3 &translation) noexcept;
    void setRotation(const glm::quat &rotation) noexcept;
    void setScale(const glm::vec3 &scale) noexcept;
    void setLocalMatrix(const glm::mat4 &matrix) noexcept;

    [[nodiscard]] const glm::vec3 &getTranslation() const noexcept;
    [[nodiscard]] const glm::quat &getRotation() const noexcept;
    [[nodiscard]] const glm::vec3 &getScale() const noexcept;
    [[nodiscard]] glm::mat4 getLocalMatrix() const noexcept;

    void addChild(std::shared_ptr<Node> child);
    [[nodiscard]] const std::vector<std::shared_ptr<Node>> &getChildren() const noexcept;
    [[nodiscard]] std::shared_ptr<Node> getParent() const noexcept;

    void updateTransforms(const glm::mat4 &parentTransform = glm::mat4(1.0f));
    [[nodiscard]] const glm::mat4 &getGlobalMatrix() const noexcept;

    void addMesh(std::shared_ptr<Mesh> mesh);
    [[nodiscard]] const std::vector<std::shared_ptr<Mesh>> &getMeshes() const noexcept;

    [[nodiscard]] const std::string &getName() const noexcept;

  private:
    std::string _name;
    glm::vec3 _translation{0.0f};
    glm::quat _rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 _scale{1.0f};
    glm::mat4 _localMatrix{1.0f};
    glm::mat4 _globalMatrix{1.0f};
    bool _isMatrixDirty{true};
    bool _hasCustomMatrix{false};

    std::weak_ptr<Node> _parent;
    std::vector<std::shared_ptr<Node>> _children;
    std::vector<std::shared_ptr<Mesh>> _meshes;
};
} // namespace bgl::gfx
