#ifndef __BGL_PNG_HPP__
#define __BGL_PNG_HPP__

#include "loader.hpp"
#include "../texture.hpp"
#include <filesystem>
#include <vector>

namespace bgl::png
{
    class Loader : public bgl::ITextureLoader
    {
    public:
        // Load a single PNG image as a 1-layer 2D Texture Array
        explicit Loader(const std::filesystem::path &path);

        // Load multiple PNG images (e.g. sequence of animation frames) into a 2D Texture Array
        explicit Loader(const std::vector<std::filesystem::path> &paths);

        ~Loader() override = default;

        GLuint upload() override;

    private:
        void loadFile(const std::filesystem::path &path);

        std::vector<ImageLayer> _layers;
    };
} // namespace bgl::png

#endif // __BGL_PNG_HPP__
