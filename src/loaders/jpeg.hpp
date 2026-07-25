// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#pragma once

#include "loader.hpp"
#include "../gfx/texture.hpp"
#include <filesystem>
#include <vector>
#include <span>

namespace bgl::jpeg
{
    class Loader final : public bgl::ITextureLoader
    {
    public:
        // Load a single JPEG image as a 1-layer 2D Texture Array
        explicit Loader(const std::filesystem::path &path);

        // Load multiple JPEG images into a 2D Texture Array
        explicit Loader(std::span<const std::filesystem::path> paths);

        ~Loader() override = default;

        [[nodiscard]] GLuint upload() override;

    private:
        void loadFile(const std::filesystem::path &path);

        std::vector<ImageLayer> _layers;
    };
} // namespace bgl::jpeg
