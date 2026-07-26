// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#include "ShaderLoader.hpp"
#include "glad/gl.h"

#include <vector>
#include <print>
#include <filesystem>
#include <fstream>
#include <string>
#include <span>

#ifdef BGL_ENABLE_GLSL_LOADER
static GLuint compileGLSLShader(GLenum type, std::string_view source)
{
    GLuint shader = glCreateShader(type);
    const char *src = source.data();

    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> infoLog(static_cast<size_t>(length));
        glGetShaderInfoLog(shader, length, nullptr, infoLog.data());

        std::println("GLSL shader compilation error ({}):\n{}",
                     (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"),
                     infoLog.data());

        glDeleteShader(shader);
        return 0;
    }
    std::println("GLSL shader compiled successfully ({}).", (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"));
    return shader;
}
#endif

static GLuint compileSPIRVShader(GLenum type, std::span<const uint32_t> spirvBinary)
{
    GLuint shader = glCreateShader(type);
    
    glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, spirvBinary.data(), static_cast<GLsizei>(spirvBinary.size_bytes()));

    glSpecializeShaderARB(shader, "main", 0, nullptr, nullptr);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> infoLog(static_cast<size_t>(length));
        glGetShaderInfoLog(shader, length, nullptr, infoLog.data());

        std::println("SPIR-V shader specialization error ({}):\n{}",
                     (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"),
                     infoLog.data());

        glDeleteShader(shader);
        return 0;
    }
    std::println("SPIR-V shader specialized successfully ({}).", (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"));
    return shader;
}

namespace bgl::io
{
#ifdef BGL_ENABLE_GLSL_LOADER
    std::string LoadShaderFromFile(const std::filesystem::path &path)
    {
        std::ifstream file(path, std::ios::in);
        if (!file.is_open())
        {
            std::println("Error: Could not open GLSL shader file: {}", path.string());
            return "";
        }

        file.seekg(0, std::ios::end);
        const std::streampos size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::string buffer(static_cast<size_t>(size), ' ');
        file.read(buffer.data(), size);
        return buffer;
    }

    GLuint CreateShaderProgramFromGLSL(std::string_view vertexSrc, std::string_view fragmentSrc)
    {
        std::println("Creating shader program from GLSL source...");
        const GLuint vertexShader = compileGLSLShader(GL_VERTEX_SHADER, vertexSrc);
        const GLuint fragmentShader = compileGLSLShader(GL_FRAGMENT_SHADER, fragmentSrc);

        if (vertexShader == 0 || fragmentShader == 0)
        {
            return 0;
        }

        const GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        GLint success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            GLint length = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            std::vector<char> infoLog(static_cast<size_t>(length));
            glGetProgramInfoLog(program, length, nullptr, infoLog.data());

            std::println("GLSL shader program linking error:\n{}", infoLog.data());

            glDeleteProgram(program);
            return 0;
        }

        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        std::println("GLSL shader program created successfully.");
        return program;
    }
#endif

    std::vector<uint32_t> LoadSPIRVShaderFromFile(const std::filesystem::path &path)
    {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            std::println("Error: Could not open SPIR-V shader file: {}", path.string());
            return {};
        }

        const size_t fileSize = static_cast<size_t>(file.tellg());
        if (fileSize % sizeof(uint32_t) != 0) {
            std::println("Error: SPIR-V file size is not a multiple of 4: {}", path.string());
            return {};
        }
        std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(fileSize));

        std::println("Loaded SPIR-V file: {} ({} bytes)", path.string(), fileSize);
        return buffer;
    }

    GLuint CreateShaderProgramFromSPIRV(std::span<const uint32_t> vertexSpv, std::span<const uint32_t> fragmentSpv)
    {
        std::println("Creating shader program from SPIR-V binaries...");
        const GLuint vertexShader = compileSPIRVShader(GL_VERTEX_SHADER, vertexSpv);
        const GLuint fragmentShader = compileSPIRVShader(GL_FRAGMENT_SHADER, fragmentSpv);

        if (vertexShader == 0 || fragmentShader == 0)
        {
            return 0;
        }

        const GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        GLint success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            GLint length = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            std::vector<char> infoLog(static_cast<size_t>(length));
            glGetProgramInfoLog(program, length, nullptr, infoLog.data());

            std::println("SPIR-V shader program linking error:\n{}", infoLog.data());

            glDeleteProgram(program);
            return 0;
        }

        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        std::println("SPIR-V shader program created successfully.");
        return program;
    }

} // namespace bgl::io
