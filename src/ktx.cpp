#include "ktx.hpp"

#include "glad/gl.h" // KRITISCH: Muss vor ktx.h kommen, damit glGenTextures etc. bekannt sind!
#include <ktx.h>

#include <spdlog/spdlog.h>
#include <unordered_set>
#include <string>
#include <filesystem>
#include <stdexcept>

#include <ktx.h>
GLuint UploadArray(ktxTexture2 *_texture, ktx_transcode_fmt_e _targetFormat); // from  texture.cpp

namespace bgl::ktx
{
    constexpr const char *glErrorString(GLenum error)
    {
        switch (error)
        {
        case GL_NO_ERROR:
            return "GL_NO_ERROR";
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        default:
            return "UNKNOWN_GL_ERROR";
        }
    }

    ktx_transcode_fmt_e GetTextureFormat(
        const std::unordered_set<std::string> &extensions,
        bool hasAlpha,
        bool isNormalMap = false)
    {
        // 1. S3TC (DXT1, DXT3, DXT5 / BC1, BC2, BC3)
        if (extensions.contains("GL_EXT_texture_compression_s3tc"))
        {
            return hasAlpha ? KTX_TTF_BC3_RGBA : KTX_TTF_BC1_RGB;
        }

        // 2. DXT1 explizit (BC1)
        if (extensions.contains("GL_EXT_texture_compression_dxt1"))
        {
            return KTX_TTF_BC1_RGB;
        }

        // 3. RGTC (BC4, BC5)
        // Optimal für reine Daten-Texturen, um S3TC-Artefakte zu vermeiden.
        if (extensions.contains("GL_EXT_texture_compression_rgtc"))
        {
            return isNormalMap ? KTX_TTF_BC5_RG : KTX_TTF_BC4_R;
        }

        // 4. Absoluter Fallback (unkomprimiert)
        return hasAlpha ? KTX_TTF_RGBA32 : KTX_TTF_RGB565;
    }

    Loader::Loader(const std::filesystem::path &path, ktx_transcode_fmt_e targetFormat)
        : _targetFormat(targetFormat)
    {
        try
        {
            load(path);
        }
        catch (const std::exception &e)
        {
            ktxTexture2_Destroy(_texture);
            throw;
        }

        // print num  layers, mipmaps yes, no
        spdlog::info("num layers: {}", _texture->numLayers);
        spdlog::info("num levels: {}", _texture->numLevels);
    }

    Loader::~Loader()
    {
        ktxTexture2_Destroy(_texture);
    }

    //[2026-07-09 12:27:49.257] [info] num layers: 13
    //[2026-07-09 12:27:49.257] [info] num levels: 10
    // Could not load OpenGL command: glBindTexture!
    //[2026-07-09 12:27:49.284] [error] failed to upload KTX texture. KTX error: Metadata key or loader-required GPU function not found., GL error: GL_NO_ERROR
    // task: Failed to run task "run": exit status 1

    GLuint Loader::upload()
    {

        return UploadArray(_texture, _targetFormat);

        GLuint glTextureId = 0;
        GLenum target = 0;
        GLenum glError = 0;

        const KTX_error_code result = ktxTexture_GLUpload(ktxTexture(_texture), &glTextureId, &target, &glError);
        if (result != KTX_SUCCESS)
        {
            auto message = std::format("failed to upload KTX texture. KTX error: {}, GL error: {}", ktxErrorString(result), glErrorString(glError));
            throw std::runtime_error(message);
        }

        return glTextureId;
    }

    void Loader::load(const std::filesystem::path &path)
    {
        KTX_error_code result = ktxTexture2_CreateFromNamedFile(path.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &_texture);
        if (result != KTX_SUCCESS)
        {
            throw std::runtime_error("failed to load KTX file");
        }

        spdlog::info("loaded KTX file: {}", path.string());

        if (ktxTexture2_NeedsTranscoding(_texture))
        {
            transcode();
        }
    }

    void Loader::transcode()
    {
        const size_t compressedSize = _texture->dataSize;

        const auto result = ktxTexture2_TranscodeBasis(_texture, _targetFormat, 0);
        if (result != KTX_SUCCESS)
        {
            throw std::runtime_error("failed to transcode KTX file");
        }

        const auto uncompressedSize = _texture->dataSize;
        spdlog::info("transcoded KTX file");
        spdlog::info("  compressed size: {} MB", compressedSize / (1024 * 1024));
        spdlog::info("uncompressed size: {} MB", uncompressedSize / (1024 * 1024));
    }

} // bgl::ktx