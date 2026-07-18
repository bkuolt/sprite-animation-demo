#version 450 core

// Das Attribut aus unserem DSA-Setup
layout(location = 0) in vec2 aPos;

// Uniforms
layout(location = 3) uniform int u_FrameIndex;
layout(location = 4) uniform float u_TweenFactor;

// Outputs für den Fragment Shader
layout(location = 0) out vec2 v_TexCoords;
layout(location = 1) flat out int v_FrameIndex; // 'flat' ist wichtig bei Integern (keine Interpolation über das Dreieck)
layout(location = 2) out float v_TweenFactor;

void main() {
    // UV-Koordinaten generieren: Shift von [-0.5, 0.5] auf [0.0, 1.0]
    v_TexCoords = aPos + 0.5;
    v_TexCoords.y = 1 - v_TexCoords.y;

    // Uniforms durchreichen
    v_FrameIndex = u_FrameIndex;
    v_TweenFactor = u_TweenFactor;
    
    // Position setzen
    gl_Position = vec4(aPos*1.5, 0.0, 1.0);
}