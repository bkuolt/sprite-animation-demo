#ifndef __BGL_KTX_HPP__
#define __BGL_KTX_HPP__

#include <ktx.h>
#include <filesystem>

namespace bgl::ktx
{
    class Loader
    {
    public:
        Loader(const std::filesystem::path &path, ktx_transcode_fmt_e targetFormat);
        virtual ~Loader();

        GLuint upload();

    private:
        void load(const std::filesystem::path &path);
        void transcode();

        ktxTexture2 *_texture{};
        const ktx_transcode_fmt_e _targetFormat;
    };

} // bgl::ktx

#endif // __BGL_KTX_HPP__   