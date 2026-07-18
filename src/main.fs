#version 450 core

layout(location = 0) in vec2 v_TexCoords;
layout(location = 1) flat in int v_FrameIndex;
layout(location = 2) in float v_TweenFactor;

layout(location = 0) out vec4 FragColor;

// A texture array is perfect for holding multiple frames in VRAM
layout(binding = 0) uniform sampler2DArray u_TextureArray; 

void main() {
    // Sample the current frame
    vec4 currentFrame = texture(u_TextureArray, vec3(v_TexCoords, float(v_FrameIndex)));
    
    // Sample the next frame, wrapping around if it's the last one
    int nextFrameIndex = (v_FrameIndex + 1) % textureSize(u_TextureArray, 0).z;

    vec4 nextFrame = texture(u_TextureArray, vec3(v_TexCoords, float(nextFrameIndex)));
    
    // Linear interpolation between the two frames based on the tween factor
    FragColor = mix(currentFrame, nextFrame, v_TweenFactor);
}