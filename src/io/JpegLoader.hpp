// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#pragma once

#include "../gfx/Texture2DArray.hpp"
#include "TextureLoader.hpp"
#include <filesystem>
#include <span>
#include <vector>

namespace bgl::io
{
class JpegLoader final : public bgl::io::ITextureLoader
{
  public:
    // Load a single JPEG image as a 1-layer 2D Texture Array
    explicit JpegLoader(const std::filesystem::path &path);

    // Load multiple JPEG images into a 2D Texture Array
    explicit JpegLoader(std::span<const std::filesystem::path> paths);

    ~JpegLoader() override = default;

    [[nodiscard]] std::unique_ptr<bgl::gfx::Texture2DArray> upload() override;

  private:
    void loadFile(const std::filesystem::path &path);

    std::vector<bgl::gfx::ImageLayer> _layers;
};
} // namespace bgl::io
