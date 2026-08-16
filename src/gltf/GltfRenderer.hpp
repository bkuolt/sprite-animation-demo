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
 * @brief OpenGL 4.6 DSA renderer for glTF scenes implementing Physically Based Rendering (PBR).
 * Utilizes a Cook-Torrance BRDF workflow with support for BaseColor, Metallic/Roughness, Normal, Emissive, and Occlusion maps.
 */
class GltfRenderer
{
  public:
    GltfRenderer();
    ~GltfRenderer();

    /**
     * @brief Updates the scene graph for animations and dynamic transformations.
     * @param scene The glTF scene to update.
     * @param deltaTime The time elapsed since the last update.
     */
    void update(const std::shared_ptr<Scene> &scene, float deltaTime);

    /**
     * @brief Renders the glTF scene using the PBR shader pipeline.
     * @param scene The glTF scene to render.
     * @param view The 4x4 View matrix.
     * @param projection The 4x4 Projection matrix.
     * @param cameraPos The world-space camera position (used for view-dependent lighting).
     */
    void render(const std::shared_ptr<Scene> &scene, const glm::mat4 &view, const glm::mat4 &projection, const glm::vec3 &cameraPos);

  private:
    void renderNode(const std::shared_ptr<Node> &node, const glm::mat4 &view, const glm::mat4 &projection);
    GLuint compileShader(GLenum type, const char *source);
    GLuint createProgram(const char *vertSrc, const char *fragSrc);

    GLuint _program{0};
    GLint _uProgram{0};
    GLint _uModelLoc{-1};
    GLint _uViewLoc{-1};
    GLint _uProjLoc{-1};
    GLint _uNormalMatLoc{-1};
    GLint _uBaseColorFactorLoc{-1};
    GLint _uHasBaseColorTexLoc{-1};
    GLint _uBaseColorTexLoc{-1};

    GLint _uEmissiveFactorLoc{-1};
    GLint _uMetallicFactorLoc{-1};
    GLint _uRoughnessFactorLoc{-1};
    GLint _uMetallicRoughnessTexLoc{-1};
    GLint _uNormalTexLoc{-1};
    GLint _uEmissiveTexLoc{-1};
    GLint _uOcclusionTexLoc{-1};
    GLint _uHasMetallicRoughnessTexLoc{-1};
    GLint _uHasNormalTexLoc{-1};
    GLint _uHasEmissiveTexLoc{-1};
    GLint _uHasOcclusionTexLoc{-1};

    GLint _uLightPosLoc{-1};
    GLint _uLightColorLoc{-1};
    GLint _uViewPosLoc{-1};
};
} // namespace bgl::gfx
