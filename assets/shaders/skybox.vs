#version 460 core
layout(location = 0) in vec2 aPos;
out vec3 viewDirection;

uniform mat4 invViewProj;

void main()
{
    gl_Position = vec4(aPos, 0.9999, 1.0);
    vec4 target = invViewProj * vec4(aPos, 1.0, 1.0);
    viewDirection = target.xyz / target.w;
}
