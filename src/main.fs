#version 450 core

in vec2 v_TexCoords;
flat in int v_FrameIndex;
in float v_TweenFactor;

out vec4 FragColor;

// Ein Texture-Array ist perfekt, um mehrere Frames im VRAM zu halten
uniform sampler2DArray u_TextureArray; 

void main() {
    // Aktuellen Frame sampeln
    vec4 currentFrame = texture(u_TextureArray, vec3(v_TexCoords, float(v_FrameIndex)));
    
    // Nächsten Frame sampeln
    int nextFrameIndex = (v_FrameIndex + 1) % textureSize(u_TextureArray, 0).z;

    vec4 nextFrame = texture(u_TextureArray, vec3(v_TexCoords, float(nextFrameIndex)));
    
    // Lineare Interpolation zwischen den beiden Frames basierend auf dem Tween-Faktor
    FragColor = mix(currentFrame, nextFrame, v_TweenFactor);
}