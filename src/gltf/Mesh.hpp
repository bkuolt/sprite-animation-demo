// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace bgl::gl { class Texture2DArray; }

namespace bgl::gfx
{
/**
 * @brief Submesh primitive holding OpenGL buffer handles and material settings.
 */
struct Primitive
{
    GLuint vao{0};
    GLuint vbo{0};
    GLuint ebo{0};
    GLenum mode{GL_TRIANGLES};
    GLsizei count{0};
    GLenum indexType{GL_UNSIGNED_INT};
    std::size_t indexOffset{0};

    // Material properties
    glm::vec4 baseColorFactor{1.0f};
    glm::vec3 emissiveFactor{0.0f};
    GLuint baseColorTexture{0};
    GLuint metallicRoughnessTexture{0};
    GLuint normalTexture{0};
    GLuint emissiveTexture{0};
    GLuint occlusionTexture{0};
    float metallicFactor{1.0f};
    float roughnessFactor{1.0f};
    bool doubleSided{false};
    // Local AABB bounding box for culling
    glm::vec3 aabbMin{-1.0f};
    glm::vec3 aabbMax{1.0f};
};

/**
 * @brief Encapsulates OpenGL handles and material references for a glTF Mesh.
 */
class Mesh
{
  public:
    explicit Mesh(std::string name = "");
    ~Mesh();

    Mesh(const Mesh &) = delete;
    Mesh &operator=(const Mesh &) = delete;
    Mesh(Mesh &&other) noexcept;
    Mesh &operator=(Mesh &&other) noexcept;

    void addPrimitive(Primitive primitive);
    [[nodiscard]] const std::vector<Primitive> &getPrimitives() const noexcept;
    [[nodiscard]] const std::string &getName() const noexcept;

  private:
    std::string _name;
    std::vector<Primitive> _primitives;
};
} // namespace bgl::gfx
