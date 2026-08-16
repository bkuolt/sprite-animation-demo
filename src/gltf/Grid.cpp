// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "Grid.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace bgl::gfx
{
static const char *gridVertSource = R"(
#version 460 core

const vec3 pos[4] = vec3[4](
    vec3(-1.0, -1.0, 0.0),
    vec3( 1.0, -1.0, 0.0),
    vec3(-1.0,  1.0, 0.0),
    vec3( 1.0,  1.0, 0.0)
);

uniform mat4 uView;
uniform mat4 uProjection;

out vec3 nearPoint;
out vec3 farPoint;

vec3 unprojectPoint(float x, float y, float z, mat4 view, mat4 proj) {
    mat4 invVP = inverse(proj * view);
    vec4 unprojectedPoint = invVP * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main() {
    vec3 p = pos[gl_VertexID];
    nearPoint = unprojectPoint(p.x, p.y, 0.0, uView, uProjection);
    farPoint = unprojectPoint(p.x, p.y, 1.0, uView, uProjection);
    gl_Position = vec4(p, 1.0);
}
)";

static const char *gridFragSource = R"(
#version 460 core

in vec3 nearPoint;
in vec3 farPoint;

out vec4 FragColor;

uniform mat4 uView;
uniform mat4 uProjection;

vec4 grid(vec3 fragPos3D, float scale) {
    vec2 coord = fragPos3D.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1.0);
    float minimumx = min(derivative.x, 1.0);

    vec4 color = vec4(0.4, 0.4, 0.4, 1.0 - min(line, 1.0));

    // X axis (Red)
    if(fragPos3D.z > -0.1 * minimumz && fragPos3D.z < 0.1 * minimumz)
        color.rgb = vec3(0.8, 0.2, 0.2);
    // Z axis (Blue)
    if(fragPos3D.x > -0.1 * minimumx && fragPos3D.x < 0.1 * minimumx)
        color.rgb = vec3(0.2, 0.2, 0.8);

    return color;
}

float computeDepth(vec3 pos) {
    vec4 clip_space_pos = uProjection * uView * vec4(pos.xyz, 1.0);
    return (clip_space_pos.z / clip_space_pos.w) * 0.5 + 0.5;
}

void main() {
    float t = -nearPoint.y / (farPoint.y - nearPoint.y);
    if (t < 0.0) discard;

    vec3 fragPos3D = nearPoint + t * (farPoint - nearPoint);

    gl_FragDepth = computeDepth(fragPos3D);

    vec4 gridColor = grid(fragPos3D, 1.0) * float(t > 0.0);
    float linearDepth = computeDepth(fragPos3D);
    float fading = max(0.0, (0.99 - linearDepth));

    gridColor.a *= fading;
    FragColor = gridColor;
}
)";

Grid::Grid()
{
    glCreateVertexArrays(1, &_vao);

    auto compileShader = [](GLenum type, const char *src) -> GLuint {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint ok = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok)
        {
            char log[512];
            glGetShaderInfoLog(shader, 512, nullptr, log);
            glDeleteShader(shader);
            throw std::runtime_error(std::string("Grid shader compile error: ") + log);
        }
        return shader;
    };

    const GLuint vert = compileShader(GL_VERTEX_SHADER, gridVertSource);
    const GLuint frag = compileShader(GL_FRAGMENT_SHADER, gridFragSource);

    _program = glCreateProgram();
    glAttachShader(_program, vert);
    glAttachShader(_program, frag);
    glLinkProgram(_program);

    // Shaders are no longer needed after linking.
    glDeleteShader(vert);
    glDeleteShader(frag);

    GLint ok = 0;
    glGetProgramiv(_program, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[512];
        glGetProgramInfoLog(_program, 512, nullptr, log);
        glDeleteProgram(_program);
        _program = 0;
        throw std::runtime_error(std::string("Grid program link error: ") + log);
    }

    _uViewLoc = glGetUniformLocation(_program, "uView");
    _uProjLoc = glGetUniformLocation(_program, "uProjection");
}

Grid::~Grid()
{
    if (_vao) glDeleteVertexArrays(1, &_vao);
    if (_program) glDeleteProgram(_program);
}

void Grid::render(const glm::mat4 &view, const glm::mat4 &projection)
{
    if (!_program || !_vao) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(_program);
    glUniformMatrix4fv(_uViewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(_uProjLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisable(GL_BLEND);
}
} // namespace bgl::gfx
