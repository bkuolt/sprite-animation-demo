// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include <glad/gl.h>
#include <filesystem>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <memory>
#include <vector>

#include "QuadMesh.hpp"
#include "Texture2DArray.hpp"

namespace bgl::gfx
{
class TileMap
{
  public:
    TileMap() = default;

    void setTexture(std::shared_ptr<Texture2DArray> texture);
    void loadLevel(int width, int height, const std::vector<int> &data);
    bool loadFromFile(const std::filesystem::path &jsonPath);

    void render(GLuint programID, const QuadMesh &quad, const glm::mat4 &projection);

    [[nodiscard]] int getWidth() const { return m_width; }
    [[nodiscard]] int getHeight() const { return m_height; }

  private:
    std::shared_ptr<Texture2DArray> m_texture;
    int m_width{0};
    int m_height{0};
    std::vector<int> m_data;
};
} // namespace bgl::gfx
