#version 460 core
// SPDX-License-Identifier: MIT

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoord;

uniform mat4 projection;

void main()
{
    TexCoord = aTexCoords;
    // Scale up the background quad so it fills the screen (for example, screen width/height based on projection or just draw a fullscreen quad)
    // If the quad is rendered via ortho, we just project it
    gl_Position = projection * vec4(aPos, 1.0);
}
