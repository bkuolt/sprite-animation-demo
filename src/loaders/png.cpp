#include "png.hpp"

#include <png.h>
#include <spdlog/spdlog.h>
#include <cstdio>
#include <stdexcept>

namespace bgl::png
{
    Loader::Loader(const std::filesystem::path &path)
    {
        loadFile(path);
    }

    Loader::Loader(const std::vector<std::filesystem::path> &paths)
    {
        for (const auto &p : paths)
        {
            loadFile(p);
        }
    }

    GLuint Loader::upload()
    {
        return UploadRawArray(_layers, true);
    }

    void Loader::loadFile(const std::filesystem::path &path)
    {
        FILE *fp = std::fopen(path.string().c_str(), "rb");
        if (!fp)
        {
            throw std::runtime_error(fmt::format("Failed to open PNG file: {}", path.string()));
        }

        unsigned char header[8];
        if (std::fread(header, 1, 8, fp) != 8 || png_sig_cmp(header, 0, 8))
        {
            std::fclose(fp);
            throw std::runtime_error(fmt::format("File is not a valid PNG: {}", path.string()));
        }

        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png_ptr)
        {
            std::fclose(fp);
            throw std::runtime_error("png_create_read_struct failed");
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
        {
            png_destroy_read_struct(&png_ptr, nullptr, nullptr);
            std::fclose(fp);
            throw std::runtime_error("png_create_info_struct failed");
        }

        if (setjmp(png_jmpbuf(png_ptr)))
        {
            png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
            std::fclose(fp);
            throw std::runtime_error(fmt::format("Error reading PNG file: {}", path.string()));
        }

        png_init_io(png_ptr, fp);
        png_set_sig_bytes(png_ptr, 8);
        png_read_info(png_ptr, info_ptr);

        png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
        png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
        png_byte color_type = png_get_color_type(png_ptr, info_ptr);
        png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

        if (bit_depth == 16)
            png_set_strip_16(png_ptr);

        if (color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_palette_to_rgb(png_ptr);

        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
            png_set_expand_gray_1_2_4_to_8(png_ptr);

        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
            png_set_tRNS_to_alpha(png_ptr);

        if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);

        if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png_ptr);

        png_read_update_info(png_ptr, info_ptr);

        ImageLayer layer;
        layer.width = width;
        layer.height = height;
        layer.channels = 4;
        layer.data.resize(width * height * 4);

        std::vector<png_bytep> row_pointers(height);
        for (png_uint_32 y = 0; y < height; ++y)
        {
            row_pointers[y] = layer.data.data() + y * width * 4;
        }

        png_read_image(png_ptr, row_pointers.data());

        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        std::fclose(fp);

        spdlog::info("Loaded PNG image: {} ({}x{})", path.string(), width, height);
        _layers.push_back(std::move(layer));
    }
} // namespace bgl::png
