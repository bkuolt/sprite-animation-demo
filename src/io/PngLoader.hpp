// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "../gfx/Texture2DArray.hpp"
#include "TextureLoader.hpp"
#include <filesystem>
#include <span>
#include <vector>

namespace bgl::io
{
class PngLoader final : public bgl::io::ITextureLoader
{
  public:
    // Load a single PNG image as a 1-layer 2D Texture Array
    explicit PngLoader(const std::filesystem::path &path);
    explicit PngLoader(std::span<const std::byte> memoryBuffer);
    explicit PngLoader(std::span<const std::filesystem::path> paths);

    ~PngLoader() override = default;

    [[nodiscard]] std::unique_ptr<bgl::gfx::Texture2DArray> upload() override;

  private:
    void loadFile(const std::filesystem::path &path);

    std::vector<bgl::gfx::ImageLayer> _layers;
};
} // namespace bgl::io
