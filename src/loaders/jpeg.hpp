#ifndef __BGL_JPEG_HPP__
#define __BGL_JPEG_HPP__

#include "loader.hpp"
#include "../texture.hpp"
#include <filesystem>
#include <vector>

namespace bgl::jpeg
{
    class Loader : public bgl::ITextureLoader
    {
    public:
        // Load a single JPEG image as a 1-layer 2D Texture Array
        explicit Loader(const std::filesystem::path &path);

        // Load multiple JPEG images into a 2D Texture Array
        explicit Loader(const std::vector<std::filesystem::path> &paths);

        ~Loader() override = default;

        GLuint upload() override;

    private:
        void loadFile(const std::filesystem::path &path);

        std::vector<ImageLayer> _layers;
    };
} // namespace bgl::jpeg

#endif // __BGL_JPEG_HPP__
