#version 450 core

layout(location = 0) in vec2 v_TexCoord;
layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D u_TextTexture;

void main() {
    FragColor = texture(u_TextTexture, v_TexCoord);
}
