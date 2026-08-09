// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "GltfRenderer.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace bgl::gfx
{
static const char *vertShaderSource = R"(
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;

void main()
{
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    FragPos = vec3(worldPos);
    Normal = uNormalMatrix * aNormal;
    TexCoord = aTexCoord;
    gl_Position = uProjection * uView * worldPos;
}
)";

static const char *fragShaderSource = R"(
#version 460 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec4 uBaseColorFactor;
uniform sampler2DArray uBaseColorTexture;
uniform bool uHasBaseColorTexture;

uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform vec3 uViewPos;

void main()
{
    vec4 baseColor = uBaseColorFactor;
    if (uHasBaseColorTexture)
    {
        baseColor *= texture(uBaseColorTexture, vec3(TexCoord, 0.0));
    }

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(uLightPos - FragPos);

    // Ambient
    float ambientStrength = 0.35;
    vec3 ambient = ambientStrength * uLightColor;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;

    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * uLightColor;

    vec3 result = (ambient + diffuse + specular) * baseColor.rgb;
    FragColor = vec4(result, baseColor.a);
}
)";

GltfRenderer::GltfRenderer()
{
    _program = createProgram(vertShaderSource, fragShaderSource);

    _uModelLoc = glGetUniformLocation(_program, "uModel");
    _uViewLoc = glGetUniformLocation(_program, "uView");
    _uProjLoc = glGetUniformLocation(_program, "uProjection");
    _uNormalMatLoc = glGetUniformLocation(_program, "uNormalMatrix");
    _uBaseColorFactorLoc = glGetUniformLocation(_program, "uBaseColorFactor");
    _uHasBaseColorTexLoc = glGetUniformLocation(_program, "uHasBaseColorTexture");
    _uBaseColorTexLoc = glGetUniformLocation(_program, "uBaseColorTexture");
    _uLightPosLoc = glGetUniformLocation(_program, "uLightPos");
    _uLightColorLoc = glGetUniformLocation(_program, "uLightColor");
    _uViewPosLoc = glGetUniformLocation(_program, "uViewPos");
}

GltfRenderer::~GltfRenderer()
{
    if (_program)
    {
        glDeleteProgram(_program);
    }
}

void GltfRenderer::update(const std::shared_ptr<Scene> &scene, float deltaTime)
{
    (void)deltaTime;
    if (scene)
    {
        scene->updateTransforms();
    }
}

void GltfRenderer::render(const std::shared_ptr<Scene> &scene, const glm::mat4 &view, const glm::mat4 &projection, const glm::vec3 &cameraPos)
{
    if (!scene || !_program) return;

    glUseProgram(_program);

    glUniformMatrix4fv(_uViewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(_uProjLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glm::vec3 lightPos(5.0f, 10.0f, 5.0f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

    glUniform3fv(_uLightPosLoc, 1, glm::value_ptr(lightPos));
    glUniform3fv(_uLightColorLoc, 1, glm::value_ptr(lightColor));
    glUniform3fv(_uViewPosLoc, 1, glm::value_ptr(cameraPos));

    for (const auto &rootNode : scene->getRootNodes())
    {
        renderNode(rootNode, view, projection);
    }
}

void GltfRenderer::renderNode(const std::shared_ptr<Node> &node, const glm::mat4 &view, const glm::mat4 &projection)
{
    if (!node) return;

    glm::mat4 model = node->getGlobalMatrix();
    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(model)));

    glUniformMatrix4fv(_uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(_uNormalMatLoc, 1, GL_FALSE, glm::value_ptr(normalMat));

    for (const auto &mesh : node->getMeshes())
    {
        if (!mesh) continue;

        for (const auto &prim : mesh->getPrimitives())
        {
            glUniform4fv(_uBaseColorFactorLoc, 1, glm::value_ptr(prim.baseColorFactor));

            if (prim.baseColorTexture > 0)
            {
                glBindTextureUnit(0, prim.baseColorTexture);
                glUniform1i(_uBaseColorTexLoc, 0);
                glUniform1i(_uHasBaseColorTexLoc, GL_TRUE);
            }
            else
            {
                glUniform1i(_uHasBaseColorTexLoc, GL_FALSE);
            }

            glBindVertexArray(prim.vao);

            if (prim.ebo > 0)
            {
                glDrawElements(prim.mode, prim.count, prim.indexType, reinterpret_cast<void*>(prim.indexOffset));
            }
            else
            {
                glDrawArrays(prim.mode, 0, prim.count);
            }
        }
    }

    for (const auto &child : node->getChildren())
    {
        renderNode(child, view, projection);
    }
}

GLuint GltfRenderer::compileShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        spdlog::error("Shader compile error: {}", infoLog);
        throw std::runtime_error("Shader compilation failed");
    }
    return shader;
}

GLuint GltfRenderer::createProgram(const char *vertSrc, const char *fragSrc)
{
    GLuint vert = compileShader(GL_VERTEX_SHADER, vertSrc);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);

    GLint success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(prog, 512, nullptr, infoLog);
        spdlog::error("Program link error: {}", infoLog);
        throw std::runtime_error("Shader program linking failed");
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
    return prog;
}
} // namespace bgl::gfx
