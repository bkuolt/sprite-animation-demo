// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "PngLoader.hpp"
#include <png++/png.hpp>
#include <spdlog/spdlog.h>
#include <sstream>
#include <stdexcept>

namespace bgl::io
{
PngLoader::PngLoader(const std::filesystem::path &path)
{
    loadFile(path);
}

PngLoader::PngLoader(std::span<const std::byte> memoryBuffer)
{
    std::string stringData(reinterpret_cast<const char *>(memoryBuffer.data()), memoryBuffer.size());
    std::istringstream stream(stringData);
    try
    {
        png::image<png::rgba_pixel> image(stream);
        bgl::gfx::ImageLayer layer;
        layer.width = image.get_width();
        layer.height = image.get_height();
        layer.channels = 4;
        layer.data.resize(layer.width * layer.height * 4);

        for (uint32_t y = 0; y < layer.height; ++y)
        {
            for (uint32_t x = 0; x < layer.width; ++x)
            {
                const auto &pixel = image.get_pixel(x, y);
                size_t index = (y * layer.width + x) * 4;
                layer.data[index + 0] = pixel.red;
                layer.data[index + 1] = pixel.green;
                layer.data[index + 2] = pixel.blue;
                layer.data[index + 3] = pixel.alpha;
            }
        }
        _layers.push_back(std::move(layer));
    }
    catch (const png::error &e)
    {
        throw std::runtime_error(fmt::format("Failed to load PNG from memory: {}", e.what()));
    }
}

PngLoader::PngLoader(std::span<const std::filesystem::path> paths)
{
    for (const auto &p : paths)
    {
        loadFile(p);
    }
}

std::unique_ptr<bgl::gfx::Texture2DArray> PngLoader::upload()
{
    return std::make_unique<bgl::gfx::Texture2DArray>(_layers, true);
}

void PngLoader::loadFile(const std::filesystem::path &path)
{
    if (!std::filesystem::exists(path))
    {
        throw std::runtime_error(fmt::format("PNG file not found: {}", path.string()));
    }

    try
    {
        png::image<png::rgba_pixel> image(path.string());

        bgl::gfx::ImageLayer layer;
        layer.width = image.get_width();
        layer.height = image.get_height();
        layer.channels = 4;
        layer.data.resize(layer.width * layer.height * 4);

        for (uint32_t y = 0; y < layer.height; ++y)
        {
            for (uint32_t x = 0; x < layer.width; ++x)
            {
                const auto &pixel = image.get_pixel(x, y);
                size_t index = (y * layer.width + x) * 4;
                layer.data[index + 0] = pixel.red;
                layer.data[index + 1] = pixel.green;
                layer.data[index + 2] = pixel.blue;
                layer.data[index + 3] = pixel.alpha;
            }
        }

        spdlog::info("Loaded PNG image: {} ({}x{})", path.string(), layer.width, layer.height);
        _layers.push_back(std::move(layer));
    }
    catch (const png::error &e)
    {
        throw std::runtime_error(fmt::format("Failed to load PNG: {}. Error: {}", path.string(), e.what()));
    }
}
} // namespace bgl::io
