#version 460 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aOffset; // per-instance offset (random seed basically)

uniform mat4 projection;
uniform float time;

out vec2 TexCoords;

// Pseudo-random function
float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    // Snow particle logic in vertex shader
    float speed = 0.5 + hash(aOffset) * 0.5; // Random falling speed
    float sway = sin(time + hash(aOffset) * 10.0) * 0.2; // Sway left/right
    
    // Calculate new position based on time
    float x = aOffset.x + sway;
    float y = aOffset.y - time * speed;
    
    // Wrap around screen to a large area to allow zooming out
    x = mod(x + 20.0, 40.0) - 20.0;
    y = mod(y + 20.0, 40.0) - 20.0;
    
    // Scale particle
    float scale = 0.005 + hash(aOffset + vec2(1.0)) * 0.015;
    
    vec2 pos = aPos * scale + vec2(x, y);
    
    gl_Position = projection * vec4(pos, 0.0, 1.0);
    
    // Standard quad UVs mapping
    TexCoords = aPos * 0.5 + 0.5;
}
