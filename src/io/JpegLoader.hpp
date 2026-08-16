// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "../gl/Texture2DArray.hpp"
#include "TextureLoader.hpp"
#include <filesystem>
#include <span>
#include <vector>

namespace bgl::io
{
/**
 * @brief Texture loader implementation for JPEG images using libjpeg.
 * Decodes standard JPEG images from disk or memory buffers.
 */
class JpegLoader final : public bgl::io::ITextureLoader
{
  public:
    /**
     * @brief Constructs a loader to decode a single JPEG image from disk.
     * @param path The file path to the JPEG image.
     */
    explicit JpegLoader(const std::filesystem::path &path);
    
    /**
     * @brief Constructs a loader to decode a JPEG image from a memory buffer.
     * Useful for glTF embedded textures.
     * @param memoryBuffer The span representing raw JPEG file bytes in memory.
     */
    explicit JpegLoader(std::span<const std::byte> memoryBuffer);
    
    /**
     * @brief Constructs a loader to decode multiple JPEG images into a Texture Array.
     * @param paths A span of paths to the JPEG images.
     */
    explicit JpegLoader(std::span<const std::filesystem::path> paths);

    ~JpegLoader() override = default;

    /**
     * @brief Uploads the decoded JPEG data to the GPU and creates a Texture2DArray.
     * @return A unique pointer to the instantiated OpenGL texture object.
     */
    [[nodiscard]] std::unique_ptr<bgl::gl::Texture2DArray> upload() override;

  private:
    void loadFile(const std::filesystem::path &path);

    std::vector<bgl::gl::ImageLayer> _layers;
};
} // namespace bgl::io
