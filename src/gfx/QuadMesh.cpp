// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "QuadMesh.hpp"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>

namespace bgl
{
QuadMesh create2DQuad()
{
    QuadMesh quad;
    quad.indexCount = 6;

    constexpr std::array<glm::vec2, 4> vertices = {glm::vec2(-0.75f, -0.75f), glm::vec2(0.75f, -0.75f),
                                                   glm::vec2(0.75f, 0.75f), glm::vec2(-0.75f, 0.75f)};

    constexpr std::array<GLuint, 6> indices = {0, 1, 2, 2, 3, 0};

    glCreateVertexArrays(1, &quad.VAO);
    glCreateBuffers(1, &quad.VBO);
    glCreateBuffers(1, &quad.IBO);

    glNamedBufferStorage(quad.VBO, vertices.size() * sizeof(glm::vec2), vertices.data(), 0);
    glNamedBufferStorage(quad.IBO, indices.size() * sizeof(GLuint), indices.data(), 0);

    glVertexArrayVertexBuffer(quad.VAO, 0, quad.VBO, 0, sizeof(glm::vec2));
    glVertexArrayElementBuffer(quad.VAO, quad.IBO);

    glEnableVertexArrayAttrib(quad.VAO, 0);
    glVertexArrayAttribFormat(quad.VAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(quad.VAO, 0, 0);

    return quad;
}

QuadMesh createOverlayQuad()
{
    QuadMesh quad;
    quad.indexCount = 6;

    constexpr std::array<OverlayVertex, 4> vertices = {{{{-1.0f, -1.0f}, {0.0f, 0.0f}},
                                                        {{1.0f, -1.0f}, {1.0f, 0.0f}},
                                                        {{1.0f, 1.0f}, {1.0f, 1.0f}},
                                                        {{-1.0f, 1.0f}, {0.0f, 1.0f}}}};

    constexpr std::array<GLuint, 6> indices = {0, 1, 2, 2, 3, 0};

    glCreateVertexArrays(1, &quad.VAO);
    glCreateBuffers(1, &quad.VBO);
    glCreateBuffers(1, &quad.IBO);

    glNamedBufferStorage(quad.VBO, vertices.size() * sizeof(OverlayVertex), vertices.data(), 0);
    glNamedBufferStorage(quad.IBO, indices.size() * sizeof(GLuint), indices.data(), 0);

    glVertexArrayVertexBuffer(quad.VAO, 0, quad.VBO, 0, sizeof(OverlayVertex));
    glVertexArrayElementBuffer(quad.VAO, quad.IBO);

    glEnableVertexArrayAttrib(quad.VAO, 0);
    glVertexArrayAttribFormat(quad.VAO, 0, 2, GL_FLOAT, GL_FALSE, offsetof(OverlayVertex, pos));
    glVertexArrayAttribBinding(quad.VAO, 0, 0);

    glEnableVertexArrayAttrib(quad.VAO, 1);
    glVertexArrayAttribFormat(quad.VAO, 1, 2, GL_FLOAT, GL_FALSE, offsetof(OverlayVertex, uv));
    glVertexArrayAttribBinding(quad.VAO, 1, 0);

    return quad;
}

void destroyQuadMesh(QuadMesh &quad)
{
    if (quad.VAO != 0)
    {
        glDeleteVertexArrays(1, &quad.VAO);
        quad.VAO = 0;
    }
    if (quad.VBO != 0)
    {
        glDeleteBuffers(1, &quad.VBO);
        quad.VBO = 0;
    }
    if (quad.IBO != 0)
    {
        glDeleteBuffers(1, &quad.IBO);
        quad.IBO = 0;
    }
}

void renderQuad(const QuadMesh &quad, GLuint textureID, GLuint shaderProgram, int currentFrameIndex, float tweenFactor,
                const glm::mat4 &projection, const glm::mat4 &model)
{
    glProgramUniform1i(shaderProgram, 3, currentFrameIndex);
    glProgramUniform1f(shaderProgram, 4, tweenFactor);
    glProgramUniformMatrix4fv(shaderProgram, 5, 1, GL_FALSE, glm::value_ptr(projection));
    glProgramUniformMatrix4fv(shaderProgram, 6, 1, GL_FALSE, glm::value_ptr(model));
    glUseProgram(shaderProgram);

    glBindTextureUnit(0, textureID);
    glBindVertexArray(quad.VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(quad.indexCount), GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
    glUseProgram(0);
}

void renderTextOverlay(const QuadMesh &quad, GLuint textureID, GLuint textShaderProgram, uint32_t texWidth,
                       uint32_t texHeight, uint32_t winWidth, uint32_t winHeight, float paddingX, float paddingY)
{
    if (winWidth == 0 || winHeight == 0 || textureID == 0)
    {
        return;
    }

    const float scaleX = static_cast<float>(texWidth) / static_cast<float>(winWidth);
    const float scaleY = static_cast<float>(texHeight) / static_cast<float>(winHeight);

    const float posX = -1.0f + 2.0f * (paddingX / static_cast<float>(winWidth)) + scaleX;
    const float posY = 1.0f - 2.0f * (paddingY / static_cast<float>(winHeight)) - scaleY;

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(posX, posY, 0.0f)) *
                          glm::scale(glm::mat4(1.0f), glm::vec3(scaleX, scaleY, 1.0f));

    glProgramUniformMatrix4fv(textShaderProgram, 5, 1, GL_FALSE, glm::value_ptr(transform));
    glUseProgram(textShaderProgram);

    glBindTextureUnit(0, textureID);
    glBindVertexArray(quad.VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(quad.indexCount), GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
    glUseProgram(0);
}
} // namespace bgl
