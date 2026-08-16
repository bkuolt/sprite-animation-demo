// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "QuadMesh.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>

namespace bgl
{
// --- QuadMesh RAII ---

QuadMesh::~QuadMesh()
{
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_ibo) { glDeleteBuffers(1, &m_ibo); m_ibo = 0; }
}

QuadMesh::QuadMesh(QuadMesh &&other) noexcept
    : m_vao(other.m_vao), m_vbo(other.m_vbo), m_ibo(other.m_ibo), m_indexCount(other.m_indexCount)
{
    other.m_vao = other.m_vbo = other.m_ibo = 0;
    other.m_indexCount = 0;
}

QuadMesh &QuadMesh::operator=(QuadMesh &&other) noexcept
{
    if (this != &other)
    {
        if (m_vao) glDeleteVertexArrays(1, &m_vao);
        if (m_vbo) glDeleteBuffers(1, &m_vbo);
        if (m_ibo) glDeleteBuffers(1, &m_ibo);

        m_vao = other.m_vao; m_vbo = other.m_vbo; m_ibo = other.m_ibo;
        m_indexCount = other.m_indexCount;
        other.m_vao = other.m_vbo = other.m_ibo = 0;
        other.m_indexCount = 0;
    }
    return *this;
}

// --- Factories ---

QuadMesh create2DQuad()
{
    constexpr std::array<glm::vec2, 4> vertices = {
        glm::vec2(-0.75f, -0.75f), glm::vec2(0.75f, -0.75f),
        glm::vec2(0.75f,  0.75f),  glm::vec2(-0.75f, 0.75f)
    };
    constexpr std::array<GLuint, 6> indices = {0, 1, 2, 2, 3, 0};

    QuadMesh quad;
    quad.m_indexCount = 6;

    glCreateVertexArrays(1, &quad.m_vao);
    glCreateBuffers(1, &quad.m_vbo);
    glCreateBuffers(1, &quad.m_ibo);

    glNamedBufferStorage(quad.m_vbo, vertices.size() * sizeof(glm::vec2), vertices.data(), 0);
    glNamedBufferStorage(quad.m_ibo, indices.size() * sizeof(GLuint), indices.data(), 0);

    glVertexArrayVertexBuffer(quad.m_vao, 0, quad.m_vbo, 0, sizeof(glm::vec2));
    glVertexArrayElementBuffer(quad.m_vao, quad.m_ibo);

    glEnableVertexArrayAttrib(quad.m_vao, 0);
    glVertexArrayAttribFormat(quad.m_vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(quad.m_vao, 0, 0);

    return quad;
}

QuadMesh createOverlayQuad()
{
    constexpr std::array<OverlayVertex, 4> vertices = {{
        {{-1.0f, -1.0f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f}, {1.0f, 0.0f}},
        {{ 1.0f,  1.0f}, {1.0f, 1.0f}},
        {{-1.0f,  1.0f}, {0.0f, 1.0f}}
    }};
    constexpr std::array<GLuint, 6> indices = {0, 1, 2, 2, 3, 0};

    QuadMesh quad;
    quad.m_indexCount = 6;

    glCreateVertexArrays(1, &quad.m_vao);
    glCreateBuffers(1, &quad.m_vbo);
    glCreateBuffers(1, &quad.m_ibo);

    glNamedBufferStorage(quad.m_vbo, vertices.size() * sizeof(OverlayVertex), vertices.data(), 0);
    glNamedBufferStorage(quad.m_ibo, indices.size() * sizeof(GLuint), indices.data(), 0);

    glVertexArrayVertexBuffer(quad.m_vao, 0, quad.m_vbo, 0, sizeof(OverlayVertex));
    glVertexArrayElementBuffer(quad.m_vao, quad.m_ibo);

    glEnableVertexArrayAttrib(quad.m_vao, 0);
    glVertexArrayAttribFormat(quad.m_vao, 0, 2, GL_FLOAT, GL_FALSE, offsetof(OverlayVertex, pos));
    glVertexArrayAttribBinding(quad.m_vao, 0, 0);

    glEnableVertexArrayAttrib(quad.m_vao, 1);
    glVertexArrayAttribFormat(quad.m_vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(OverlayVertex, uv));
    glVertexArrayAttribBinding(quad.m_vao, 1, 0);

    return quad;
}

// --- Render helpers ---

// Uniform binding layout (GLSL layout(location=N)):
//   3 = currentFrame, 4 = tweenFactor, 5 = projection, 6 = model
void renderQuad(const QuadMesh &quad, GLuint textureID, GLuint shaderProgram,
                int currentFrameIndex, float tweenFactor,
                const glm::mat4 &projection, const glm::mat4 &model)
{
    glProgramUniform1i(shaderProgram, 3, currentFrameIndex);
    glProgramUniform1f(shaderProgram, 4, tweenFactor);
    glProgramUniformMatrix4fv(shaderProgram, 5, 1, GL_FALSE, glm::value_ptr(projection));
    glProgramUniformMatrix4fv(shaderProgram, 6, 1, GL_FALSE, glm::value_ptr(model));
    glUseProgram(shaderProgram);

    glBindTextureUnit(0, textureID);
    glBindVertexArray(quad.m_vao);
    glDrawElements(GL_TRIANGLES, quad.m_indexCount, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
    glUseProgram(0);
}

void renderTextOverlay(const QuadMesh &quad, GLuint textureID, GLuint textShaderProgram,
                       uint32_t texWidth, uint32_t texHeight,
                       uint32_t winWidth, uint32_t winHeight,
                       float paddingX, float paddingY)
{
    if (winWidth == 0 || winHeight == 0 || textureID == 0)
    {
        return;
    }

    const float scaleX = static_cast<float>(texWidth)  / static_cast<float>(winWidth);
    const float scaleY = static_cast<float>(texHeight) / static_cast<float>(winHeight);

    const float posX = -1.0f + 2.0f * (paddingX / static_cast<float>(winWidth))  + scaleX;
    const float posY =  1.0f - 2.0f * (paddingY / static_cast<float>(winHeight)) - scaleY;

    const glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), glm::vec3(posX, posY, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(scaleX, scaleY, 1.0f));

    glProgramUniformMatrix4fv(textShaderProgram, 5, 1, GL_FALSE, glm::value_ptr(transform));
    glUseProgram(textShaderProgram);

    glBindTextureUnit(0, textureID);
    glBindVertexArray(quad.m_vao);
    glDrawElements(GL_TRIANGLES, quad.m_indexCount, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
    glUseProgram(0);
}
} // namespace bgl
