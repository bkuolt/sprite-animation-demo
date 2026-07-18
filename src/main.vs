#version 450 core

// The attribute from our DSA setup
layout(location = 0) in vec2 aPos;

// Uniforms
layout(location = 3) uniform int u_FrameIndex;
layout(location = 4) uniform float u_TweenFactor;

// Outputs for the Fragment Shader
layout(location = 0) out vec2 v_TexCoords;
layout(location = 1) flat out int v_FrameIndex; // 'flat' is important for integers (no interpolation across the triangle)
layout(location = 2) out float v_TweenFactor;

void main() {
    // Generate UV coordinates: Shift from [-0.5, 0.5] to [0.0, 1.0]
    v_TexCoords = aPos + 0.5;
    v_TexCoords.y = 1.0 - v_TexCoords.y; // Flip Y for standard texture coordinates

    // Pass-through uniforms
    v_FrameIndex = u_FrameIndex;
    v_TweenFactor = u_TweenFactor;
    
    // Set position
    gl_Position = vec4(aPos*1.5, 0.0, 1.0);
}