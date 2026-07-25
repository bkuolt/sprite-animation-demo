// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#pragma once

#include "loader.hpp"
#include <ktx.h>
#include <filesystem>

namespace bgl::ktx
{
    /**
     * @brief Loader for KTX 2.0 container textures using Basis Universal transcoding.
     */
    class Loader final : public bgl::ITextureLoader
    {
    public:
        Loader(const std::filesystem::path &path, ktx_transcode_fmt_e targetFormat);
        ~Loader() override;

        [[nodiscard]] GLuint upload() override;

    private:
        void load(const std::filesystem::path &path);
        void transcode();

        ktxTexture2 *_texture{nullptr};
        ktx_transcode_fmt_e _targetFormat;
    };
} // namespace bgl::ktx