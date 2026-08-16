// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "Scene.hpp"
#include <filesystem>
#include <memory>

namespace bgl::io
{
/**
 * @brief Fast glTF/GLB loader utilizing mmap via fastgltf and existing BGL texture loaders.
 */
class GltfLoader
{
  public:
    GltfLoader() = default;
    ~GltfLoader() = default;

    /**
     * @brief Loads a glTF scene from file using memory mapping.
     * @param path Path to the glTF or GLB file.
     * @return Root Scene containing the parsed scene graph.
     */
    [[nodiscard]] std::shared_ptr<bgl::gfx::Scene> loadFromFile(const std::filesystem::path &path);
};
} // namespace bgl::io
