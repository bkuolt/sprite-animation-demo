#include "glad/gl.h"
#include "shader.hpp"

#include <vector>
#include <iostream>
#include <print>

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

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

        std::cerr << "Shader Compilation Error ("
                  << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << "):\n"
                  << infoLog.data() << "\n";

        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

namespace bgl
{

    std::string LoadShaderFromFile(const std::string &filepath)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            std::cerr << "Fehler: Konnte Shader-Datei nicht öffnen: " << filepath << "\n";
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
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