#version 460 core
// SPDX-License-Identifier: MIT

layout (location = 0) in vec2 aPos;

out vec2 TexCoord;

uniform mat4 projection;

void main()
{
    // Scale quad to be massive so it acts as an infinite background
    vec2 worldPos = aPos * 100.0;
    
    // Generate UVs from the world position
    TexCoord = worldPos;
    
    gl_Position = projection * vec4(worldPos, 0.0, 1.0);
}
