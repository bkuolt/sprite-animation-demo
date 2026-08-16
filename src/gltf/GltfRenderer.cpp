// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "GltfRenderer.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace bgl::gfx
{
static const char *vertShaderSource = R"(
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out mat3 TBN;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;

void main()
{
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    FragPos = vec3(worldPos);
    Normal = uNormalMatrix * aNormal;
    TexCoord = aTexCoord;
    gl_Position = uProjection * uView * worldPos;
}
)";

static const char *fragShaderSource = R"(
#version 460 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

// Material Properties
uniform vec4 uBaseColorFactor;
uniform vec3 uEmissiveFactor;
uniform float uMetallicFactor;
uniform float uRoughnessFactor;

// Textures
uniform sampler2DArray uBaseColorTexture;
uniform sampler2DArray uMetallicRoughnessTexture;
uniform sampler2DArray uNormalTexture;
uniform sampler2DArray uEmissiveTexture;
uniform sampler2DArray uOcclusionTexture;

// Texture presence
uniform bool uHasBaseColorTexture;
uniform bool uHasMetallicRoughnessTexture;
uniform bool uHasNormalTexture;
uniform bool uHasEmissiveTexture;
uniform bool uHasOcclusionTexture;

// Lighting
uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform vec3 uViewPos;

const float PI = 3.14159265359;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(uNormalTexture, vec3(TexCoord, 0.0)).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(FragPos);
    vec3 Q2  = dFdy(FragPos);
    vec2 st1 = dFdx(TexCoord);
    vec2 st2 = dFdy(TexCoord);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.0000001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    vec4 baseColor = uBaseColorFactor;
    if (uHasBaseColorTexture) {
        baseColor *= texture(uBaseColorTexture, vec3(TexCoord, 0.0));
    }
    // discard transparent pixels for cutout materials (basic check)
    if (baseColor.a < 0.1) discard;

    float metallic = uMetallicFactor;
    float roughness = uRoughnessFactor;
    if (uHasMetallicRoughnessTexture) {
        vec4 mr = texture(uMetallicRoughnessTexture, vec3(TexCoord, 0.0));
        metallic *= mr.b;
        roughness *= mr.g;
    }

    vec3 N = uHasNormalTexture ? getNormalFromMap() : normalize(Normal);
    vec3 V = normalize(uViewPos - FragPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, baseColor.rgb, metallic);

    // Light calculation
    vec3 L = normalize(uLightPos - FragPos);
    vec3 H = normalize(V + L);
    
    // Attenuation
    float distance = length(uLightPos - FragPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = uLightColor * attenuation * 300.0; // Scaled up light intensity

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
       
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  

    float NdotL = max(dot(N, L), 0.0);        
    vec3 Lo = (kD * baseColor.rgb / PI + specular) * radiance * NdotL;

    // Ambient lighting (very simple fallback since we don't have IBL)
    vec3 ambient = vec3(0.03) * baseColor.rgb * (uHasOcclusionTexture ? texture(uOcclusionTexture, vec3(TexCoord, 0.0)).r : 1.0);
    
    // Emissive
    vec3 emissive = uEmissiveFactor;
    if (uHasEmissiveTexture) {
        emissive *= texture(uEmissiveTexture, vec3(TexCoord, 0.0)).rgb;
    }

    vec3 color = ambient + Lo + emissive;
    
    // HDR tonemapping (Exposure)
    color = vec3(1.0) - exp(-color * 1.0);
    // Gamma correction
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, baseColor.a);
}
)";

GltfRenderer::GltfRenderer()
{
    _program = createProgram(vertShaderSource, fragShaderSource);

    _uModelLoc = glGetUniformLocation(_program, "uModel");
    _uViewLoc = glGetUniformLocation(_program, "uView");
    _uProjLoc = glGetUniformLocation(_program, "uProjection");
    _uNormalMatLoc = glGetUniformLocation(_program, "uNormalMatrix");
    _uBaseColorFactorLoc = glGetUniformLocation(_program, "uBaseColorFactor");
    _uHasBaseColorTexLoc = glGetUniformLocation(_program, "uHasBaseColorTexture");
    _uBaseColorTexLoc = glGetUniformLocation(_program, "uBaseColorTexture");
    
    _uEmissiveFactorLoc = glGetUniformLocation(_program, "uEmissiveFactor");
    _uMetallicFactorLoc = glGetUniformLocation(_program, "uMetallicFactor");
    _uRoughnessFactorLoc = glGetUniformLocation(_program, "uRoughnessFactor");
    
    _uMetallicRoughnessTexLoc = glGetUniformLocation(_program, "uMetallicRoughnessTexture");
    _uNormalTexLoc = glGetUniformLocation(_program, "uNormalTexture");
    _uEmissiveTexLoc = glGetUniformLocation(_program, "uEmissiveTexture");
    _uOcclusionTexLoc = glGetUniformLocation(_program, "uOcclusionTexture");

    _uHasMetallicRoughnessTexLoc = glGetUniformLocation(_program, "uHasMetallicRoughnessTexture");
    _uHasNormalTexLoc = glGetUniformLocation(_program, "uHasNormalTexture");
    _uHasEmissiveTexLoc = glGetUniformLocation(_program, "uHasEmissiveTexture");
    _uHasOcclusionTexLoc = glGetUniformLocation(_program, "uHasOcclusionTexture");

    _uLightPosLoc = glGetUniformLocation(_program, "uLightPos");
    _uLightColorLoc = glGetUniformLocation(_program, "uLightColor");
    _uViewPosLoc = glGetUniformLocation(_program, "uViewPos");
}

GltfRenderer::~GltfRenderer()
{
    if (_program)
    {
        glDeleteProgram(_program);
    }
}

void GltfRenderer::update(const std::shared_ptr<Scene> &scene, float deltaTime)
{
    (void)deltaTime;
    if (scene)
    {
        scene->updateTransforms();
    }
}

void GltfRenderer::render(const std::shared_ptr<Scene> &scene, const glm::mat4 &view, const glm::mat4 &projection, const glm::vec3 &cameraPos)
{
    if (!scene || !_program) return;

    glUseProgram(_program);

    glUniformMatrix4fv(_uViewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(_uProjLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glm::vec3 lightPos(5.0f, 10.0f, 5.0f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

    glUniform3fv(_uLightPosLoc, 1, glm::value_ptr(lightPos));
    glUniform3fv(_uLightColorLoc, 1, glm::value_ptr(lightColor));
    glUniform3fv(_uViewPosLoc, 1, glm::value_ptr(cameraPos));

    for (const auto &rootNode : scene->getRootNodes())
    {
        renderNode(rootNode, view, projection);
    }
}

void GltfRenderer::renderNode(const std::shared_ptr<Node> &node, const glm::mat4 &view, const glm::mat4 &projection)
{
    if (!node) return;

    glm::mat4 model = node->getGlobalMatrix();
    glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(model)));

    glUniformMatrix4fv(_uModelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(_uNormalMatLoc, 1, GL_FALSE, glm::value_ptr(normalMat));

    for (const auto &mesh : node->getMeshes())
    {
        if (!mesh) continue;

        for (const auto &prim : mesh->getPrimitives())
        {
            glUniform4fv(_uBaseColorFactorLoc, 1, glm::value_ptr(prim.baseColorFactor));
            glUniform3fv(_uEmissiveFactorLoc, 1, glm::value_ptr(prim.emissiveFactor));
            glUniform1f(_uMetallicFactorLoc, prim.metallicFactor);
            glUniform1f(_uRoughnessFactorLoc, prim.roughnessFactor);

            int texUnit = 0;

            auto bindTex = [&](GLuint tex, GLint loc, GLint hasLoc, const char* name) {
                if (tex > 0) {
                    if (!glIsTexture(tex)) {
                        spdlog::error("Texture {} (name: {}) is NOT a valid texture object!", tex, name);
                    }
                    glBindTextureUnit(texUnit, tex);
                    glUniform1i(loc, texUnit);
                    glUniform1i(hasLoc, GL_TRUE);
                    texUnit++;
                } else {
                    glUniform1i(hasLoc, GL_FALSE);
                }
            };

            bindTex(prim.baseColorTexture, _uBaseColorTexLoc, _uHasBaseColorTexLoc, "BaseColor");
            bindTex(prim.metallicRoughnessTexture, _uMetallicRoughnessTexLoc, _uHasMetallicRoughnessTexLoc, "MetallicRoughness");
            bindTex(prim.normalTexture, _uNormalTexLoc, _uHasNormalTexLoc, "Normal");
            bindTex(prim.emissiveTexture, _uEmissiveTexLoc, _uHasEmissiveTexLoc, "Emissive");
            bindTex(prim.occlusionTexture, _uOcclusionTexLoc, _uHasOcclusionTexLoc, "Occlusion");

            glBindVertexArray(prim.vao);

            if (prim.ebo > 0)
            {
                glDrawElements(prim.mode, prim.count, prim.indexType, reinterpret_cast<void*>(prim.indexOffset));
            }
            else
            {
                glDrawArrays(prim.mode, 0, prim.count);
            }
        }
    }

    for (const auto &child : node->getChildren())
    {
        renderNode(child, view, projection);
    }
}

GLuint GltfRenderer::compileShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        spdlog::error("Shader compile error: {}", infoLog);
        throw std::runtime_error("Shader compilation failed");
    }
    return shader;
}

GLuint GltfRenderer::createProgram(const char *vertSrc, const char *fragSrc)
{
    GLuint vert = compileShader(GL_VERTEX_SHADER, vertSrc);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);

    GLint success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(prog, 512, nullptr, infoLog);
        spdlog::error("Program link error: {}", infoLog);
        throw std::runtime_error("Shader program linking failed");
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
    return prog;
}
} // namespace bgl::gfx
