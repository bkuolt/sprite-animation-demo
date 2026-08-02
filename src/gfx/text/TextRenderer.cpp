// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "TextRenderer.hpp"

#include <algorithm>
#include <cmath>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace bgl
{

GLuint TextRenderer::s_ssboProgram = 0;
GLuint TextRenderer::s_ssboBuffer = 0;
GLuint TextRenderer::s_emptyVao = 0;
GLuint TextRenderer::s_fbo = 0;

TextTexture::TextTexture(GLuint textureId, uint32_t width, uint32_t height)
    : m_textureId(textureId), m_width(width), m_height(height)
{
}

TextTexture::~TextTexture()
{
    Cleanup();
}

TextTexture::TextTexture(TextTexture &&other) noexcept
    : m_textureId(other.m_textureId), m_width(other.m_width), m_height(other.m_height)
{
    other.m_textureId = 0;
    other.m_width = 0;
    other.m_height = 0;
}

TextTexture &TextTexture::operator=(TextTexture &&other) noexcept
{
    if (this != &other)
    {
        Cleanup();

        m_textureId = other.m_textureId;
        m_width = other.m_width;
        m_height = other.m_height;

        other.m_textureId = 0;
        other.m_width = 0;
        other.m_height = 0;
    }
    return *this;
}

void TextTexture::Cleanup() noexcept
{
    if (m_textureId != 0)
    {
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
        m_width = 0;
        m_height = 0;
    }
}

void TextTexture::Bind(uint32_t slot) const
{
    if (m_textureId != 0)
    {
        glBindTextureUnit(slot, m_textureId);
    }
}

static GLuint CompileShader(GLenum type, const char *src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        spdlog::error("Shader compile error: {}", infoLog);
        throw std::runtime_error("Failed to compile text quad SSBO shader");
    }
    return shader;
}

void TextRenderer::InitSsboPipeline()
{
    if (s_ssboProgram != 0)
        return;

    const char *vertSrc = R"(#version 450 core

struct QuadInstance {
    vec4 pos;   // x0, y0, x1, y1
    vec4 uv;    // u0, v0, u1, v1
    vec4 color; // r, g, b, a
};

layout(std430, binding = 0) readonly buffer QuadBuffer {
    QuadInstance quads[];
};

uniform vec2 u_TargetSize;

out vec2 v_UV;
out vec4 v_Color;

void main() {
    uint quadIdx = gl_VertexID / 6;
    uint vertIdx = gl_VertexID % 6;

    QuadInstance q = quads[quadIdx];

    vec2 pos;
    vec2 uv;

    if (vertIdx == 0) { pos = vec2(q.pos.x, q.pos.y); uv = vec2(q.uv.x, q.uv.y); }
    else if (vertIdx == 1) { pos = vec2(q.pos.z, q.pos.y); uv = vec2(q.uv.z, q.uv.y); }
    else if (vertIdx == 2) { pos = vec2(q.pos.z, q.pos.w); uv = vec2(q.uv.z, q.uv.w); }
    else if (vertIdx == 3) { pos = vec2(q.pos.z, q.pos.w); uv = vec2(q.uv.z, q.uv.w); }
    else if (vertIdx == 4) { pos = vec2(q.pos.x, q.pos.w); uv = vec2(q.uv.x, q.uv.w); }
    else if (vertIdx == 5) { pos = vec2(q.pos.x, q.pos.y); uv = vec2(q.uv.x, q.uv.y); }

    vec2 ndc = (pos / u_TargetSize) * 2.0 - 1.0;
    ndc.y = -ndc.y;

    gl_Position = vec4(ndc, 0.0, 1.0);
    v_UV = uv;
    v_Color = q.color;
}
)";

    const char *fragSrc = R"(#version 450 core

in vec2 v_UV;
in vec4 v_Color;

out vec4 FragColor;

layout(binding = 0) uniform sampler2D u_Atlas;

void main() {
    vec4 texColor = texture(u_Atlas, v_UV);
    FragColor = v_Color * texColor;
}
)";

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vertSrc);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragSrc);

    s_ssboProgram = glCreateProgram();
    glAttachShader(s_ssboProgram, vs);
    glAttachShader(s_ssboProgram, fs);
    glLinkProgram(s_ssboProgram);

    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint status = 0;
    glGetProgramiv(s_ssboProgram, GL_LINK_STATUS, &status);
    if (!status)
    {
        char infoLog[512];
        glGetProgramInfoLog(s_ssboProgram, sizeof(infoLog), nullptr, infoLog);
        spdlog::error("Program link error: {}", infoLog);
        throw std::runtime_error("Failed to link text quad SSBO shader program");
    }

    glCreateBuffers(1, &s_ssboBuffer);
    glCreateVertexArrays(1, &s_emptyVao);
    glCreateFramebuffers(1, &s_fbo);
}

TextTexture TextRenderer::RenderToTexture(const Font &font, std::string_view text, glm::u8vec4 textColor,
                                          glm::u8vec4 backgroundColor, uint32_t padding)
{
    ShapedText shapedText = TextShaper::ShapeText(font, text);
    return RenderShapedToTexture(font, shapedText, textColor, backgroundColor, padding);
}

TextTexture TextRenderer::RenderShapedToTexture(const Font &font, const ShapedText &shapedText, glm::u8vec4 textColor,
                                                glm::u8vec4 backgroundColor, uint32_t padding)
{
    InitSsboPipeline();

    // Generate or use texture atlas for font
    TextureAtlas atlas = GenerateTextureAtlas(font);
    if (!atlas.IsValid())
    {
        throw std::runtime_error("Failed to acquire valid font texture atlas");
    }

    const uint32_t textWidth = static_cast<uint32_t>(shapedText.width);
    const uint32_t textHeight = static_cast<uint32_t>(shapedText.height);

    const uint32_t totalWidth = textWidth + padding * 2;
    const uint32_t totalHeight = textHeight + padding * 2;

    const int32_t baselineY = static_cast<int32_t>(padding) + shapedText.maxAscent;
    int32_t currentX = static_cast<int32_t>(padding);

    glm::vec4 normTextColor = {textColor.r / 255.0f, textColor.g / 255.0f, textColor.b / 255.0f, textColor.a / 255.0f};

    std::vector<TextQuadInstance> quads;
    quads.reserve(shapedText.glyphs.size());

    for (const auto &glyph : shapedText.glyphs)
    {
        const GlyphAtlasInfo *gInfo = atlas.GetGlyph(glyph.codepoint);
        if (!gInfo || gInfo->width == 0 || gInfo->height == 0)
        {
            currentX += glyph.xAdvance;
            continue;
        }

        float x0 = static_cast<float>(currentX + glyph.xOffset + gInfo->bearingX);
        float y0 = static_cast<float>(baselineY - gInfo->bearingY - glyph.yOffset);
        float x1 = x0 + static_cast<float>(gInfo->width);
        float y1 = y0 + static_cast<float>(gInfo->height);

        TextQuadInstance quad{};
        quad.pos = glm::vec4(x0, y0, x1, y1);
        quad.uv = glm::vec4(gInfo->u0, gInfo->v0, gInfo->u1, gInfo->v1);
        quad.color = normTextColor;

        quads.push_back(quad);
        currentX += glyph.xAdvance;
    }

    // Create target texture for rendering text with mipmaps
    GLuint targetTexture = 0;
    glCreateTextures(GL_TEXTURE_2D, 1, &targetTexture);
    GLsizei mipLevels = static_cast<GLsizei>(1 + std::floor(std::log2(std::max(totalWidth, totalHeight))));
    glTextureStorage2D(targetTexture, mipLevels, GL_RGBA8, totalWidth, totalHeight);

    glTextureParameteri(targetTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(targetTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(targetTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(targetTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Save previous state
    GLint prevFbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);

    // Bind offscreen FBO and attach target texture
    glNamedFramebufferTexture(s_fbo, GL_COLOR_ATTACHMENT0, targetTexture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, s_fbo);
    glViewport(0, 0, totalWidth, totalHeight);

    // Clear background
    float bgR = backgroundColor.r / 255.0f;
    float bgG = backgroundColor.g / 255.0f;
    float bgB = backgroundColor.b / 255.0f;
    float bgA = backgroundColor.a / 255.0f;
    glClearColor(bgR, bgG, bgB, bgA);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!quads.empty())
    {
        // Upload quad instances to SSBO buffer
        glNamedBufferData(s_ssboBuffer, quads.size() * sizeof(TextQuadInstance), quads.data(), GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, s_ssboBuffer);

        // Bind atlas texture to slot 0
        atlas.Bind(0);

        glUseProgram(s_ssboProgram);
        const GLint sizeLoc = glGetUniformLocation(s_ssboProgram, "u_TargetSize");
        glUniform2f(sizeLoc, static_cast<float>(totalWidth), static_cast<float>(totalHeight));

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindVertexArray(s_emptyVao);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(quads.size() * 6));
    }

    // Generate Mipmaps for the rendered text texture
    glGenerateTextureMipmap(targetTexture);

    // Restore previous state
    glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);

    spdlog::info("Rendered SSBO Quad Text Texture ID {} ({}x{}, {} quads, mipmapped)", targetTexture, totalWidth,
                 totalHeight, quads.size());

    return TextTexture(targetTexture, totalWidth, totalHeight);
}

} // namespace bgl
