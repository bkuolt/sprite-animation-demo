#version 460 core

in vec2 TexCoords;
out vec4 FragColor;

layout(binding = 1) uniform sampler2D snowTexture;

void main()
{
    vec4 texColor = texture(snowTexture, TexCoords);
    if (texColor.a < 0.1)
        discard;
    FragColor = texColor;
}
