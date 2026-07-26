// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "sampler.hpp"
#include <spdlog/spdlog.h>

namespace bgl::gfx
{
Sampler::Sampler()
{
    glCreateSamplers(1, &m_handle);
    spdlog::debug("Created Sampler with GL ID: {}", m_handle);
}

Sampler::~Sampler()
{
    cleanup();
}

Sampler::Sampler(Sampler &&other) noexcept : m_handle(other.m_handle)
{
    other.m_handle = 0;
}

Sampler &Sampler::operator=(Sampler &&other) noexcept
{
    if (this != &other)
    {
        cleanup();
        m_handle = other.m_handle;
        other.m_handle = 0;
    }
    return *this;
}

void Sampler::cleanup() noexcept
{
    if (m_handle != 0)
    {
        glDeleteSamplers(1, &m_handle);
        spdlog::debug("Deleted Sampler GL ID: {}", m_handle);
        m_handle = 0;
    }
}

void Sampler::setFilter(GLenum minFilter, GLenum magFilter)
{
    glSamplerParameteri(m_handle, GL_TEXTURE_MIN_FILTER, minFilter);
    glSamplerParameteri(m_handle, GL_TEXTURE_MAG_FILTER, magFilter);
}

void Sampler::setWrap(GLenum wrapS, GLenum wrapT, GLenum wrapR)
{
    glSamplerParameteri(m_handle, GL_TEXTURE_WRAP_S, wrapS);
    glSamplerParameteri(m_handle, GL_TEXTURE_WRAP_T, wrapT);
    glSamplerParameteri(m_handle, GL_TEXTURE_WRAP_R, wrapR);
}

void Sampler::setAnisotropy(float maxAnisotropy)
{
    // Note: GL_TEXTURE_MAX_ANISOTROPY became core in 4.6
    glSamplerParameterf(m_handle, GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy);
}

void Sampler::bind(GLuint textureUnit) const
{
    glBindSampler(textureUnit, m_handle);
}
} // namespace bgl::gfx
