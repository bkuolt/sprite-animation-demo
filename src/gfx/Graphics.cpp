// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "Graphics.hpp"
#include <spdlog/spdlog.h>

namespace bgl
{
std::unique_ptr<bgl::gl::Sampler> CreateDefaultSampler()
{
    auto sampler = std::make_unique<bgl::gl::Sampler>();
    sampler->setFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    sampler->setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

    if (GLAD_GL_EXT_texture_filter_anisotropic)
    {
        float maxAnisotropy = 1.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        sampler->setAnisotropy(maxAnisotropy);
        spdlog::info("Enabled Anisotropic Filtering (Max: {})", maxAnisotropy);
    }
    else if (GLAD_GL_VERSION_4_6)
    {
        float maxAnisotropy = 1.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy);
        sampler->setAnisotropy(maxAnisotropy);
        spdlog::info("Enabled Anisotropic Filtering (Core 4.6, Max: {})", maxAnisotropy);
    }

    return sampler;
}
} // namespace bgl
