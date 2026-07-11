#include "glad/gl.h"
#include "shader.hpp"

#include <vector>
#include <print>

#include <fstream>
#include <string>

static GLuint compileShader(GLenum type, std::string_view source)
{
    GLuint shader = glCreateShader(type);
    const char *src = source.data();

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

        std::print("Shader Compilation Error ({}):\n{}\n",
                   (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"),
                   infoLog.data());

        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

namespace bgl
{

    std::string LoadShaderFromFile(const std::string &path)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            print("Failed to open shader file: {}", path);
            return "";
        }

        file.seekg(0, std::ios::end);
        std::streampos size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        file.read(buffer.data(), size);
        return std::string(buffer.data(), size);
    }

    GLuint CreateShaderProgram(std::string_view vertexSrc, std::string_view fragmentSrc)
    {
        GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSrc);
        GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);

        // Abbruch, falls ein Shader nicht kompiliert werden konnte
        if (vertexShader == 0 || fragmentShader == 0)
        {
            return 0;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        // Error Checking für das Linking
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            GLint length;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            std::vector<char> infoLog(length);
            glGetProgramInfoLog(program, length, nullptr, infoLog.data());

            std::print("Log: {}", std::string(infoLog.data()));

            glDeleteProgram(program);
            return 0;
        }

        // Cleanup: Nach dem erfolgreichen Linken brauchen wir die einzelnen Shader-Objekte nicht mehr
        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }

} // namespace bgl