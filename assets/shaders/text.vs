#version 450 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

layout(location = 0) out vec2 v_TexCoord;

layout(location = 5) uniform mat4 u_Transform;

void main() {
    v_TexCoord = aTexCoord;
    gl_Position = u_Transform * vec4(aPos, 0.0, 1.0);
}
