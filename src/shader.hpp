#ifndef SHADER_HPP
#define SHADER_HPP

#include "glad/gl.h"
#include <string_view>

namespace bgl
{

    std::string LoadShaderFromFile(const std::string &filepath);
    GLuint CreateShaderProgram(std::string_view vertexSrc, std::string_view fragmentSrc);
}

#endif // SHADER_HPP