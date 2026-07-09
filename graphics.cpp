#include "glad/gl.h"
#include "KHR/khrplatform.h"
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>
#include <stdexcept>
#include <fmt/ranges.h> // fmt::join

#include <glm/vec2.hpp>

namespace
{

    void DebugCallback(GLenum source,
                       GLenum type,
                       GLuint id,
                       GLenum severity,
                       GLsizei length,
                       const GLchar *message,
                       const void *userParam)
    {
        // TODO
    }

    std::vector<std::string> getTextureCompressionExtensions()
    {
        GLint num_extensions = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
        std::vector<std::string> extensions;
        extensions.reserve(num_extensions);

        for (GLint i = 0; i < num_extensions; ++i)
        {
            const std::string ext_str((char *)glGetStringi(GL_EXTENSIONS, i));
            if (ext_str.find("GL_EXT_texture_compression_") != std::string_view::npos)
            {
                extensions.push_back(ext_str);
            }
        }

        return extensions;
    }

} // namespace

namespace bgl
{

    void InitializeGLAD()
    {
        const int gladVersion = gladLoadGL(glfwGetProcAddress);
        if (gladVersion == 0)
        {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        spdlog::info("GLAD Version: {}.{}", (int)GLAD_VERSION_MAJOR(gladVersion), (int)GLAD_VERSION_MINOR(gladVersion));
    }

    void IntitializeOpenGL()
    {
        // print OpenGL info
        const char *version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
        spdlog::info("OpenGL Version: {}", version);
        spdlog::info("Vendor: {}", (const char *)glGetString(GL_VENDOR));
        spdlog::info("Renderer: {}", (const char *)glGetString(GL_RENDERER));
        spdlog::info("GLSL Version: {}", (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION));

        // setup debug callbacks
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(DebugCallback, nullptr);

        // handle extensions
        auto extensions = getTextureCompressionExtensions();
        spdlog::info("Extensions: {}", fmt::join(extensions, ", "));

        if (!(GL_ARB_texture_compression ||
              GL_EXT_texture_compression_s3tc ||
              GL_ARB_texture_compression_rgtc))
        {
            throw std::runtime_error("GL_ARB_texture_compression not supported!");
        }

        if (!GL_ARB_gl_spirv)
        {
            throw std::runtime_error("GL_ARB_gl_spirv not supported!");
        }
    }

    void Draw(double time)
    {
        auto c = (((int)time) % 10) / 10.0f;

        // set unifroms
        glProgramUniform1f(0, 0,0);

        glClearColor(c * 2, c, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

} // namespace bgl