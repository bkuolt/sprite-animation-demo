// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "Scene.hpp"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <memory>

namespace bgl::gfx
{
/**
 * @brief OpenGL 4.6 DSA renderer for glTF scenes with Phong shading.
 */
class GltfRenderer
{
  public:
    GltfRenderer();
    ~GltfRenderer();

    void update(const std::shared_ptr<Scene> &scene, float deltaTime);
    void render(const std::shared_ptr<Scene> &scene, const glm::mat4 &view, const glm::mat4 &projection, const glm::vec3 &cameraPos);

  private:
    void renderNode(const std::shared_ptr<Node> &node, const glm::mat4 &view, const glm::mat4 &projection);
    GLuint compileShader(GLenum type, const char *source);
    GLuint createProgram(const char *vertSrc, const char *fragSrc);

    GLuint _program{0};
    GLint _uModelLoc{-1};
    GLint _uViewLoc{-1};
    GLint _uProjLoc{-1};
    GLint _uNormalMatLoc{-1};
    GLint _uBaseColorFactorLoc{-1};
    GLint _uHasBaseColorTexLoc{-1};
    GLint _uBaseColorTexLoc{-1};
    GLint _uLightPosLoc{-1};
    GLint _uLightColorLoc{-1};
    GLint _uViewPosLoc{-1};
};
} // namespace bgl::gfx
