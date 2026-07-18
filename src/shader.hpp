#ifndef SHADER_HPP
#define SHADER_HPP

#include "glad/gl.h"
#include <vector>
#include <string_view>
#include <filesystem> // For std::filesystem::path

namespace bgl
{

    std::string LoadShaderFromFile(const std::filesystem::path &filepath); // For GLSL source
    std::vector<uint32_t> LoadSPIRVShaderFromFile(const std::filesystem::path &filepath); // For SPIR-V binary
    GLuint CreateShaderProgramFromGLSL(std::string_view vertexSrc, std::string_view fragmentSrc);
    GLuint CreateShaderProgramFromSPIRV(const std::vector<uint32_t>& vertexSpv, const std::vector<uint32_t>& fragmentSpv);
}

#endif // SHADER_HPP