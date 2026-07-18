#include "glad/gl.h"
#include "shader.hpp"

#include <vector>
#include <print>

#include <filesystem>
#include <fstream>
#include <string>

// --- Internal helpers for GLSL compilation ---

// Compiles a GLSL shader from a source string
static GLuint compileGLSLShader(GLenum type, std::string_view source)
{
    GLuint shader = glCreateShader(type);
    const char *src = source.data();

    // Pass GLSL source code to the shader object
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    // Error Checking
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> infoLog(length);
        glGetShaderInfoLog(shader, length, nullptr, infoLog.data());

        std::print("GLSL shader compilation error ({}):\n{}\n",
                   (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"),
                   infoLog.data());

        glDeleteShader(shader);
        return 0;
    }
    std::print("GLSL shader compiled successfully ({}).\n", (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"));
    return shader;
}

// Compiles a SPIR-V shader from binary data
static GLuint compileSPIRVShader(GLenum type, const std::vector<uint32_t>& spirvBinary)
{
    GLuint shader = glCreateShader(type);
    
    // Pass SPIR-V binary data to the shader object
    glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, spirvBinary.data(), spirvBinary.size() * sizeof(uint32_t));

    // Specialize the shader (entry point "main")
    glSpecializeShaderARB(shader, "main", 0, nullptr, nullptr);

    // Error checking
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success); // GL_COMPILE_STATUS wird auch für die Spezialisierung verwendet
    if (!success)
    {
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> infoLog(length);
        glGetShaderInfoLog(shader, length, nullptr, infoLog.data());

        std::print("SPIR-V shader specialization error ({}):\n{}\n",
                   (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"),
                   infoLog.data());

        glDeleteShader(shader);
        return 0;
    }
    std::print("SPIR-V shader specialized successfully ({}).\n", (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"));
    return shader;
}

namespace bgl
{

    // --- GLSL Shader Loading and Program Creation ---

    std::string LoadShaderFromFile(const std::filesystem::path &path)
    {
        std::ifstream file(path, std::ios::in);
        if (!file.is_open())
        {
            std::print("Error: Could not open GLSL shader file: {}\n", path.string());
            return "";
        }

        file.seekg(0, std::ios::end);
        std::streampos size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::string buffer(static_cast<size_t>(size), ' ');
        file.read(buffer.data(), size); // Read directly into the string's buffer
        return buffer;
    }

    GLuint CreateShaderProgramFromGLSL(std::string_view vertexSrc, std::string_view fragmentSrc)
    {
        std::print("Creating shader program from GLSL source...\n");
        GLuint vertexShader = compileGLSLShader(GL_VERTEX_SHADER, vertexSrc);
        GLuint fragmentShader = compileGLSLShader(GL_FRAGMENT_SHADER, fragmentSrc);

        // Abort if a shader could not be compiled
        if (vertexShader == 0 || fragmentShader == 0)
        {
            return 0;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        // Error checking for linking
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            GLint length;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            std::vector<char> infoLog(length);
            glGetProgramInfoLog(program, length, nullptr, infoLog.data());

            std::print("GLSL shader program linking error:\n{}\n", infoLog.data());

            glDeleteProgram(program);
            return 0;
        }

        // Cleanup: After successful linking, we no longer need the individual shader objects
        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        std::print("GLSL shader program created successfully.\n");
        return program;
    }

    // --- SPIR-V Shader Loading and Program Creation ---

    std::vector<uint32_t> LoadSPIRVShaderFromFile(const std::filesystem::path &path)
    {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            std::print("Error: Could not open SPIR-V shader file: {}\n", path.string());
            return {};
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        // SPIR-V is an array of uint32_t, so the size must be a multiple of 4
        if (fileSize % sizeof(uint32_t) != 0) {
            std::print("Error: SPIR-V file size is not a multiple of 4: {}\n", path.string());
            return {};
        }
        std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        file.close();

        std::print("Loaded SPIR-V file: {} ({} bytes)\n", path.string(), fileSize);
        return buffer;
    }

    GLuint CreateShaderProgramFromSPIRV(const std::vector<uint32_t>& vertexSpv, const std::vector<uint32_t>& fragmentSpv)
    {
        std::print("Creating shader program from SPIR-V binaries...\n");
        GLuint vertexShader = compileSPIRVShader(GL_VERTEX_SHADER, vertexSpv);
        GLuint fragmentShader = compileSPIRVShader(GL_FRAGMENT_SHADER, fragmentSpv);

        if (vertexShader == 0 || fragmentShader == 0)
        {
            return 0;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        // Error checking for linking (same as for GLSL)
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            GLint length;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            std::vector<char> infoLog(length);
            glGetProgramInfoLog(program, length, nullptr, infoLog.data());

            std::print("SPIR-V shader program linking error:\n{}\n", infoLog.data());

            glDeleteProgram(program);
            return 0;
        }

        // Cleanup: Detach and delete individual shader objects after successful linking
        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        std::print("SPIR-V shader program created successfully.\n");
        return program;
    }

} // namespace bgl