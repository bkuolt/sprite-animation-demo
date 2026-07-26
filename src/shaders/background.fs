#version 460 core
// SPDX-License-Identifier: MIT

in vec2 TexCoord;
out vec4 FragColor;

void main()
{
    // Checkered background pattern
    float checkerSize = 20.0;
    vec2 pos = floor(TexCoord * checkerSize);
    float pattern = mod(pos.x + pos.y, 2.0);
    
    // Pattern color: dark grey and light grey
    vec3 color1 = vec3(0.2);
    vec3 color2 = vec3(0.3);
    
    vec3 finalColor = mix(color1, color2, pattern);
    
    FragColor = vec4(finalColor, 1.0);
}
