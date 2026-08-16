// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "TextureLoader.hpp"
#include <filesystem>
#include <ktx.h>

namespace bgl::io
{
/**
 * @brief Loader for KTX 2.0 container textures using Basis Universal transcoding.
 */
class KtxLoader final : public bgl::io::ITextureLoader
{
  public:
    KtxLoader(const std::filesystem::path &path, ktx_transcode_fmt_e targetFormat);
    KtxLoader(std::span<const std::byte> memoryBuffer, ktx_transcode_fmt_e targetFormat);
    ~KtxLoader() override;

    [[nodiscard]] std::unique_ptr<bgl::gl::Texture2DArray> upload() override;

  private:
    void load(const std::filesystem::path &path);
    void transcode();

    ktxTexture2 *_texture{nullptr};
    ktx_transcode_fmt_e _targetFormat;
};
} // namespace bgl::io