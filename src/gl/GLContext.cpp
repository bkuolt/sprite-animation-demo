// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "GLContext.hpp"

#include <glad/gl.h>
#include <QOpenGLContext>

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

namespace bgl::gl
{
void InitializeGLAD()
{
    auto *ctx = QOpenGLContext::currentContext();
    if (!ctx)
    {
        throw std::runtime_error("No active QOpenGLContext found when initializing GLAD");
    }

    const int gladVersion = gladLoadGL(reinterpret_cast<GLADloadfunc>(+[](const char *name) -> void * {
        auto *c = QOpenGLContext::currentContext();
        return c ? reinterpret_cast<void *>(c->getProcAddress(name)) : nullptr;
    }));

    if (gladVersion == 0)
    {
        throw std::runtime_error("Failed to initialize GLAD via Qt QOpenGLContext");
    }

    spdlog::info("GLAD Version: {}.{}", static_cast<int>(GLAD_VERSION_MAJOR(gladVersion)),
                 static_cast<int>(GLAD_VERSION_MINOR(gladVersion)));
}

void InitializeOpenGL()
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

std::optional<int> GetVRAMUsageMB()
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

    return std::nullopt; // Driver does not support VRAM queries.
}
} // namespace bgl::gl
