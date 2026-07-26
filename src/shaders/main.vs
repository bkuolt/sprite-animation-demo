#version 450 core

layout(location = 0) in vec2 aPos;

layout(location = 3) uniform int u_FrameIndex;
layout(location = 4) uniform float u_TweenFactor;
layout(location = 5) uniform mat4 u_Projection;

layout(location = 0) out vec2 v_TexCoords;
layout(location = 1) flat out int v_FrameIndex;
layout(location = 2) out float v_TweenFactor;

void main() {
    v_TexCoords = (aPos / 1.5) + 0.5;
    v_TexCoords.y = 1.0 - v_TexCoords.y;

    v_FrameIndex = u_FrameIndex;
    v_TweenFactor = u_TweenFactor;
    
    gl_Position = u_Projection * vec4(aPos, 0.0, 1.0);
}
