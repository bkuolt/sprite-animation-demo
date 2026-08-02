// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "Graphics.hpp"
#include "../Math.hpp"
#include "Sampler.hpp"

#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <fmt/ranges.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, const GLchar *message,
                   const void * /*userParam*/)
{
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    std::string_view sourceStr;
    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        sourceStr = "API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        sourceStr = "Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        sourceStr = "Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        sourceStr = "Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        sourceStr = "Application";
        break;
    default:
        sourceStr = "Other";
        break;
    }

    std::string_view typeStr;
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        typeStr = "Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        typeStr = "Deprecated Behaviour";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        typeStr = "Undefined Behaviour";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        typeStr = "Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        typeStr = "Performance";
        break;
    case GL_DEBUG_TYPE_MARKER:
        typeStr = "Marker";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        typeStr = "Push Group";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        typeStr = "Pop Group";
        break;
    default:
        typeStr = "Other";
        break;
    }

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        spdlog::critical("[OpenGL Debug] [{}] {} from {}: {}", id, typeStr, sourceStr, message);
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        spdlog::warn("[OpenGL Debug] [{}] {} from {}: {}", id, typeStr, sourceStr, message);
        break;
    case GL_DEBUG_SEVERITY_LOW:
        spdlog::info("[OpenGL Debug] [{}] {} from {}: {}", id, typeStr, sourceStr, message);
        break;
    default:
        spdlog::trace("[OpenGL Debug] [{}] {} from {}: {}", id, typeStr, sourceStr, message);
        break;
    }
}

std::vector<std::string> getTextureCompressionExtensions()
{
    GLint num_extensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    std::vector<std::string> extensions;
    extensions.reserve(static_cast<size_t>(num_extensions));

    for (GLint i = 0; i < num_extensions; ++i)
    {
        const std::string ext_str(reinterpret_cast<const char *>(glGetStringi(GL_EXTENSIONS, i)));
        if (ext_str.find("GL_EXT_texture_compression_") != std::string_view::npos)
        {
            extensions.push_back(ext_str);
        }
    }

    return extensions;
}
} // namespace

namespace bgl
{
void InitializeGLAD()
{
    const int gladVersion = gladLoadGL(glfwGetProcAddress);
    if (gladVersion == 0)
    {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    spdlog::info("GLAD Version: {}.{}", static_cast<int>(GLAD_VERSION_MAJOR(gladVersion)),
                 static_cast<int>(GLAD_VERSION_MINOR(gladVersion)));
}

void IntitializeOpenGL()
{
    const auto *version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    spdlog::info("OpenGL Version: {}", version);
    spdlog::info("Vendor: {}", reinterpret_cast<const char *>(glGetString(GL_VENDOR)));
    spdlog::info("Renderer: {}", reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    spdlog::info("GLSL Version: {}", reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(DebugCallback, nullptr);

    glEnable(GL_MULTISAMPLE);
    spdlog::info("MSAA enabled if supported by the windowing system.");

    const auto extensions = getTextureCompressionExtensions();
    spdlog::info("Extensions: {}", fmt::join(extensions, ", "));

    if (!(GL_ARB_texture_compression || GL_EXT_texture_compression_s3tc || GL_ARB_texture_compression_rgtc))
    {
        throw std::runtime_error("GL_ARB_texture_compression not supported!");
    }

    if (!GL_ARB_gl_spirv)
    {
        throw std::runtime_error("GL_ARB_gl_spirv not supported!");
    }
}

std::unique_ptr<bgl::gfx::Sampler> CreateDefaultSampler()
{
    auto sampler = std::make_unique<bgl::gfx::Sampler>();
    sampler->setFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    sampler->setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

    if (GLAD_GL_EXT_texture_filter_anisotropic)
    {
        float maxAnisotropy = 1.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        sampler->setAnisotropy(maxAnisotropy);
        spdlog::info("Enabled Anisotropic Filtering (Max: {})", maxAnisotropy);
    }
    else if (GLAD_GL_VERSION_4_6)
    {
        float maxAnisotropy = 1.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy);
        sampler->setAnisotropy(maxAnisotropy);
        spdlog::info("Enabled Anisotropic Filtering (Core 4.6, Max: {})", maxAnisotropy);
    }

    return sampler;
}

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

int GetVRAMUsageMB()
{
    GLint totalMemKb = 0;
    GLint availMemKb = 0;

    glGetIntegerv(0x9048, &totalMemKb); // GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX
    glGetIntegerv(0x9049, &availMemKb); // GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX

    if (totalMemKb > 0 && availMemKb >= 0)
    {
        return (totalMemKb - availMemKb) / 1024;
    }

    GLint freeMemAmd[4] = {0};
    glGetIntegerv(0x87FC, freeMemAmd); // TEXTURE_FREE_MEMORY_ATI
    if (freeMemAmd[0] > 0)
    {
        return freeMemAmd[0] / 1024;
    }

    return 42; // Fallback representation if driver does not support queries
}
} // namespace bgl
