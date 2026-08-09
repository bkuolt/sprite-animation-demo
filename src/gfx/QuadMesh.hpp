// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include <glad/gl.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <cstdint>

namespace bgl
{
struct OverlayVertex
{
    glm::vec2 pos;
    glm::vec2 uv;
};

/**
 * @brief RAII-compliant OpenGL 4.6 DSA quad mesh.
 *
 * Non-copyable, moveable. Automatically releases GPU resources on destruction.
 */
class QuadMesh
{
  public:
    QuadMesh() = default;
    ~QuadMesh();

    QuadMesh(const QuadMesh &) = delete;
    QuadMesh &operator=(const QuadMesh &) = delete;

    QuadMesh(QuadMesh &&other) noexcept;
    QuadMesh &operator=(QuadMesh &&other) noexcept;

    [[nodiscard]] bool isValid() const noexcept { return VAO != 0; }

    GLuint VAO{0};
    GLuint VBO{0};
    GLuint IBO{0};
    GLsizei indexCount{0};
};

[[nodiscard]] QuadMesh create2DQuad();
[[nodiscard]] QuadMesh createOverlayQuad();

void renderQuad(const QuadMesh &quad, GLuint textureID, GLuint shaderProgram, int currentFrameIndex,
                float tweenFactor, const glm::mat4 &projection, const glm::mat4 &model);

void renderTextOverlay(const QuadMesh &quad, GLuint textureID, GLuint textShaderProgram, uint32_t texWidth,
                       uint32_t texHeight, uint32_t winWidth, uint32_t winHeight, float paddingX = 15.0f,
                       float paddingY = 15.0f);
} // namespace bgl
