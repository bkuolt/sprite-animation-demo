// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#include "jpegLoader.hpp"

#include <csetjmp>
#include <cstdio>
#include <jpeglib.h>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace
{
struct CustomJpegErrorMgr
{
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

METHODDEF(void) CustomJpegErrorExit(j_common_ptr cinfo)
{
    auto *err = reinterpret_cast<CustomJpegErrorMgr *>(cinfo->err);
    (*cinfo->err->output_message)(cinfo);
    longjmp(err->setjmp_buffer, 1);
}
} // namespace

namespace bgl::io
{
JpegLoader::JpegLoader(const std::filesystem::path &path)
{
    loadFile(path);
}

JpegLoader::JpegLoader(std::span<const std::filesystem::path> paths)
{
    for (const auto &p : paths)
    {
        loadFile(p);
    }
}

std::unique_ptr<bgl::gfx::Texture2DArray> JpegLoader::upload()
{
    return std::make_unique<bgl::gfx::Texture2DArray>(_layers, true);
}

void JpegLoader::loadFile(const std::filesystem::path &path)
{
    std::unique_ptr<FILE, int (*)(FILE *)> fp(std::fopen(path.string().c_str(), "rb"), std::fclose);
    if (!fp)
    {
        throw std::runtime_error(fmt::format("Failed to open JPEG file: {}", path.string()));
    }

    struct jpeg_decompress_struct cinfo{};
    CustomJpegErrorMgr jerr{};

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = CustomJpegErrorExit;

    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
        throw std::runtime_error(fmt::format("Error reading JPEG file: {}", path.string()));
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fp.get());
    jpeg_read_header(&cinfo, TRUE);

    cinfo.out_color_space = JCS_RGB;
    jpeg_start_decompress(&cinfo);

    bgl::gfx::ImageLayer layer;
    layer.width = cinfo.output_width;
    layer.height = cinfo.output_height;
    layer.channels = cinfo.output_components;
    layer.data.resize(static_cast<size_t>(layer.width) * layer.height * layer.channels);

    const int row_stride = layer.width * layer.channels;
    JSAMPROW row_pointer[1];

    while (cinfo.output_scanline < cinfo.output_height)
    {
        row_pointer[0] = &layer.data[cinfo.output_scanline * row_stride];
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    spdlog::info("Loaded JPEG image: {} ({}x{})", path.string(), layer.width, layer.height);
    _layers.push_back(std::move(layer));
}
} // namespace bgl::io
