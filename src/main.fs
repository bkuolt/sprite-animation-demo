#version 450 core

layout(location = 0) in vec2 v_TexCoords;
layout(location = 1) flat in int v_FrameIndex;
layout(location = 2) in float v_TweenFactor;

layout(location = 0) out vec4 FragColor;

// Ein Texture-Array ist perfekt, um mehrere Frames im VRAM zu halten
layout(binding = 0) uniform sampler2DArray u_TextureArray; 

void main() {
    // Aktuellen Frame sampeln
    vec4 currentFrame = texture(u_TextureArray, vec3(v_TexCoords, float(v_FrameIndex)));
    
    // Nächsten Frame sampeln
    int nextFrameIndex = (v_FrameIndex + 1) % textureSize(u_TextureArray, 0).z;

    vec4 nextFrame = texture(u_TextureArray, vec3(v_TexCoords, float(nextFrameIndex)));
    
    // Lineare Interpolation zwischen den beiden Frames basierend auf dem Tween-Faktor
    FragColor = mix(currentFrame, nextFrame, v_TweenFactor);
}