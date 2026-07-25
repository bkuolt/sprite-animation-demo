#ifndef __BGL_LOADER_HPP__
#define __BGL_LOADER_HPP__

#include <glad/gl.h>

namespace bgl
{
    class ITextureLoader
    {
    public:
        virtual ~ITextureLoader() = default;

        // Uploads texture data to GPU and returns OpenGL texture ID (GL_TEXTURE_2D_ARRAY)
        virtual GLuint upload() = 0;
    };
} // namespace bgl

#endif // __BGL_LOADER_HPP__
