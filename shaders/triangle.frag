#version 450

// Input vom Vertex Shader
layout(location = 0) in vec3 fragColor;

// Output in den Framebuffer
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}