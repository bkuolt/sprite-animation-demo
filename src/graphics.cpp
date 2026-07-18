#include "glad/gl.h"
#include "KHR/khrplatform.h"
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>
#include <stdexcept>

#include <fmt/ranges.h> // fmt::join

#include <glm/vec2.hpp>
#include "ktx.hpp"
#include <print>

#include "shader.hpp"

extern int currentAnimation;

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
        // Ignore certain informational messages
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
            return;

        std::string_view sourceStr;
        switch (source)
        {
        case GL_DEBUG_SOURCE_API:             sourceStr = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceStr = "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceStr = "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     sourceStr = "Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           sourceStr = "Other"; break;
        }

        std::string_view typeStr;
        switch (type)
        {
        case GL_DEBUG_TYPE_ERROR:               typeStr = "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "Undefined Behaviour"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              typeStr = "Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               typeStr = "Other"; break;
        }

        switch (severity)
        {
        case GL_DEBUG_SEVERITY_HIGH:         spdlog::critical("[OpenGL Debug] [{}] {} from {}: {}", id, typeStr, sourceStr, message); break;
        case GL_DEBUG_SEVERITY_MEDIUM:       spdlog::warn("[OpenGL Debug] [{}] {} from {}: {}", id, typeStr, sourceStr, message); break;
        case GL_DEBUG_SEVERITY_LOW:          spdlog::info("[OpenGL Debug] [{}] {} from {}: {}", id, typeStr, sourceStr, message); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: spdlog::trace("[OpenGL Debug] [{}] {} from {}: {}", id, typeStr, sourceStr, message); break;
        }
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
    struct QuadMesh
    {
        GLuint VAO;
        GLuint VBO;
        GLuint IBO;
        unsigned int indexCount;
    };

    std::vector<GLuint> _textureIDs;
    QuadMesh _mesh;
    GLuint _program;
    std::once_flag _initFlag;

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

    QuadMesh create2DQuad()
    {
        QuadMesh quad;
        quad.indexCount = 6;

        std::vector<glm::vec2> vertices = {
            glm::vec2(-0.75f, -0.75f), // Unten-Links  (Index 0)
            glm::vec2(0.75f, -0.75f),  // Unten-Rechts (Index 1)
            glm::vec2(0.75f, 0.75f),   // Oben-Rechts  (Index 2)
            glm::vec2(-0.75f, 0.75f)   // Oben-Links   (Index 3)
        };

        std::vector<GLuint> indices = {
            0, 1, 2,
            2, 3, 0};

        glCreateVertexArrays(1, &quad.VAO);
        glCreateBuffers(1, &quad.VBO);
        glCreateBuffers(1, &quad.IBO);

        glNamedBufferStorage(quad.VBO, vertices.size() * sizeof(glm::vec2), vertices.data(), 0);
        glNamedBufferStorage(quad.IBO, indices.size() * sizeof(GLuint), indices.data(), 0);

        glVertexArrayVertexBuffer(quad.VAO, 0, quad.VBO, 0, sizeof(glm::vec2));
        glVertexArrayElementBuffer(quad.VAO, quad.IBO);

        glEnableVertexArrayAttrib(quad.VAO, 0);
        glVertexArrayAttribFormat(quad.VAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(quad.VAO, 0, 0);

        return quad;
    }

    void loadAssets()
    {
        auto binaryPath = std::filesystem::read_symlink("/proc/self/exe");
        auto basePath = binaryPath.parent_path();

        // Load textures
        const std::vector<std::filesystem::path> fileNames{
            basePath / "assets" / "idle.ktx2",
            basePath / "assets" / "walk.ktx2",
            basePath / "assets" / "jump.ktx2",
            basePath / "assets" / "run.ktx2",
            basePath / "assets" / "slide.ktx2",
            basePath / "assets" / "dead.ktx2"};

        _textureIDs.resize(fileNames.size());
        for (size_t i = 0; i < fileNames.size(); ++i)
        {
            bgl::ktx::Loader loader(fileNames[i], KTX_TTF_BC3_RGBA);
            _textureIDs[i] = loader.upload();
        }

        // Load shaders
        auto vsSpv = bgl::LoadSPIRVShaderFromFile(basePath / "assets" / "main.vert.spv");
        auto fsSpv = bgl::LoadSPIRVShaderFromFile(basePath / "assets" / "main.frag.spv");
        _program = bgl::CreateShaderProgramFromSPIRV(vsSpv, fsSpv);

        // Create mesh
        _mesh = create2DQuad();
    }

    void renderQuad(const QuadMesh &quad, GLuint textureID, GLuint shaderProgram, int currentFrameIndex, float tweenFactor)
    {
        // When using SPIR-V shaders with explicit layout locations for uniforms,
        // these locations can be used directly instead of calling glGetUniformLocation.
        // See main.vs: layout(location = 3) uniform int u_FrameIndex;
        // See main.vs: layout(location = 4) uniform float u_TweenFactor;
        glProgramUniform1i(shaderProgram, 3, currentFrameIndex);
        glProgramUniform1f(shaderProgram, 4, tweenFactor);
        glUseProgram(shaderProgram);

        glBindTextureUnit(0, textureID);
        glBindVertexArray(quad.VAO);
        glDrawElements(GL_TRIANGLES, quad.indexCount, GL_UNSIGNED_INT, nullptr);

        glBindVertexArray(0);
        glUseProgram(0);
    }

    void Draw(double time)
    {
        std::call_once(_initFlag, []()
                       { loadAssets(); });

        int currentTexture = currentAnimation % _textureIDs.size(); // currentAnimation is a global variable

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        const auto c = (static_cast<int>(time) % 10) / 10.0f;
        glClearColor(c * 0.2f, c * 0.1f, 0.3f, 1.0f); // A subtle background color pulse
        glClear(GL_COLOR_BUFFER_BIT);

        GLint layers = 0;
        // Get the number of layers (frames) from the texture array using DSA
        glGetTextureLevelParameteriv(_textureIDs[currentTexture], 0, GL_TEXTURE_DEPTH, &layers);


        const auto num_frames = layers > 0 ? layers : 1;
        constexpr auto target_fps = 24.0;

        const double totalFrames = time * target_fps;
        const int currentFrame = static_cast<int>(std::floor(totalFrames)) % num_frames;
        const float tweenFactor = static_cast<float>(totalFrames - std::floor(totalFrames));

        renderQuad(_mesh, _textureIDs[currentTexture], _program, currentFrame, tweenFactor);
    }

    //--------------------------------------------

} // namespace bgl