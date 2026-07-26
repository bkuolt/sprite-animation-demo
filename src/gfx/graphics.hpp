// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#pragma once

#include "glad/gl.h"
#include "glad/gl.h"
#include <cstdint>
#include <memory>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

namespace bgl
{
struct QuadMesh
{
    GLuint VAO{0};
    GLuint VBO{0};
    GLuint IBO{0};
    unsigned int indexCount{0};
};

struct OverlayVertex
{
    glm::vec2 pos;
    glm::vec2 uv;
};

namespace gfx {
    class Sampler;
}

void InitializeGLAD();
void IntitializeOpenGL();

[[nodiscard]] std::unique_ptr<gfx::Sampler> CreateDefaultSampler();

[[nodiscard]] QuadMesh create2DQuad();
[[nodiscard]] QuadMesh createOverlayQuad();
void destroyQuadMesh(QuadMesh &quad);

void renderQuad(const QuadMesh &quad, GLuint textureID, GLuint shaderProgram, int currentFrameIndex, float tweenFactor,
                const glm::mat4 &projection);
void renderTextOverlay(const QuadMesh &quad, GLuint textureID, GLuint textShaderProgram, uint32_t texWidth,
                       uint32_t texHeight, uint32_t winWidth, uint32_t winHeight, float paddingX = 15.0f,
                       float paddingY = 15.0f);

[[nodiscard]] int GetVRAMUsageMB();
} // namespace bgl
