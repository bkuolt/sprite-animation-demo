// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

namespace bgl::gfx
{
/**
 * @brief Infinite procedural 3D grid rendered on the XZ plane.
 */
class Grid
{
  public:
    Grid();
    ~Grid();

    void render(const glm::mat4 &view, const glm::mat4 &projection);

  private:
    GLuint _vao{0};
    GLuint _program{0};
    GLint _uViewLoc{-1};
    GLint _uProjLoc{-1};
};
} // namespace bgl::gfx
