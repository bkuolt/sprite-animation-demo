#version 460 core
// SPDX-License-Identifier: MIT

in vec2 TexCoord;
out vec4 FragColor;

void main()
{
    // Checkerboard pattern (smaller squares)
    float size = 30.0; 
    vec2 p = TexCoord * size;
    
    // Analytical anti-aliasing (box filter) to remove jagged edges
    vec2 dpdx = dFdx(p);
    vec2 dpdy = dFdy(p);
    vec2 w = abs(dpdx) + abs(dpdy) + 0.001; // filter width
    
    // Integral of the step function over the pixel area
    vec2 i = 2.0 * (abs(fract((p - 0.5 * w) * 0.5) - 0.5) - abs(fract((p + 0.5 * w) * 0.5) - 0.5)) / w;
    float checker = 0.5 - 0.5 * i.x * i.y;
    
    // Use grey tones instead of pure black/white
    vec3 color = mix(vec3(0.25), vec3(0.35), checker);
    
    FragColor = vec4(color, 1.0);
}
