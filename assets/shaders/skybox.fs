#version 460 core
in vec3 viewDirection;
out vec4 FragColor;

layout(binding = 0) uniform samplerCube skyboxMap;

void main()
{
    // The view direction needs to be normalized.
    vec3 dir = normalize(viewDirection);
    // Depending on the cubemap, we might need to flip Y or Z
    FragColor = texture(skyboxMap, dir);
}
