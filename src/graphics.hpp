// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#pragma once

#include "glad/gl.h"
#include <vector>
#include <filesystem>
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

    void InitializeGLAD();
    void IntitializeOpenGL();

    [[nodiscard]] QuadMesh create2DQuad();
    void renderQuad(const QuadMesh &quad, GLuint textureID, GLuint shaderProgram, int currentFrameIndex, float tweenFactor);
} // namespace bgl