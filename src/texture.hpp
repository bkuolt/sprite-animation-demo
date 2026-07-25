#ifndef __BGL_TEXTURE_HPP__
#define __BGL_TEXTURE_HPP__

#include <glad/gl.h>
#include <ktx.h>
#include <vector>
#include <cstdint>
#include <cstddef>

GLuint UploadArray(ktxTexture2 *_texture, ktx_transcode_fmt_e _targetFormat);

namespace bgl
{
    struct ImageLayer
    {
        uint32_t width{0};
        uint32_t height{0};
        uint32_t channels{4}; // 3 = RGB, 4 = RGBA
        std::vector<uint8_t> data;
    };

    GLuint UploadRawArray(const std::vector<ImageLayer> &layers, bool generateMipmaps = true);
}

#endif // __BGL_TEXTURE_HPP__
