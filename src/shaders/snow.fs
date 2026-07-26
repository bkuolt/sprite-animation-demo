#version 460 core

in vec2 TexCoords;
out vec4 FragColor;



void main()
{
    // Procedural soft circle for snowflake
    vec2 center = vec2(0.5, 0.5);
    float dist = distance(TexCoords, center);
    float alpha = 1.0 - smoothstep(0.1, 0.5, dist);
    
    if (alpha < 0.01)
        discard;
        
    FragColor = vec4(1.0, 1.0, 1.0, alpha);
}
