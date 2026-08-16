// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "TileMap.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fstream>

namespace bgl::gfx
{
void TileMap::setTexture(std::shared_ptr<bgl::gl::Texture2DArray> texture)
{
    m_texture = std::move(texture);
}

void TileMap::loadLevel(int width, int height, const std::vector<int> &data)
{
    m_width = width;
    m_height = height;
    m_data = data;
}

bool TileMap::loadFromFile(const std::filesystem::path &jsonPath)
{
    if (!std::filesystem::exists(jsonPath))
    {
        spdlog::error("TileMap JSON file does not exist: {}", jsonPath.string());
        return false;
    }

    std::ifstream file(jsonPath);
    if (!file.is_open())
    {
        spdlog::error("Failed to open TileMap JSON file: {}", jsonPath.string());
        return false;
    }

    try
    {
        nlohmann::json root;
        file >> root;

        m_width = root.value("width", 0);
        m_height = root.value("height", 0);
        m_data = root.value("data", std::vector<int>{});

        spdlog::info("Loaded TileMap level '{}' ({}x{}) from JSON",
                     root.value("name", "Unnamed"), m_width, m_height);
        return true;
    }
    catch (const std::exception &ex)
    {
        spdlog::error("Failed to parse TileMap JSON: {}", ex.what());
        return false;
    }
}

void TileMap::render(GLuint programID, const QuadMesh &quad, const glm::mat4 &projection)
{
    if (!m_texture || m_data.empty() || m_width <= 0 || m_height <= 0)
    {
        return;
    }

    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            const int tileID = m_data[y * m_width + x];
            if (tileID <= 0)
                continue;

            const int layer = tileID - 1;
            glm::vec2 pos(static_cast<float>(x), static_cast<float>(m_height - 1 - y));
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x * 2.0f, pos.y * 2.0f, 0.0f));

            renderQuad(quad, m_texture->getHandle(), programID, layer, 0.0f, projection, model);
        }
    }
}
} // namespace bgl::gfx
