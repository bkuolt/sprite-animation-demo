#include "glad/gl.h"
#include "KHR/khrplatform.h"

#include <GLFW/glfw3.h>
#include <iostream>

#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <fmt/ranges.h>
#include <spdlog/spdlog.h>
#include <fmt/core.h>

#include "ktx.hpp"

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

    void KeyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        spdlog::trace("Key {} pressed", key);

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            return;
        }
    }

    void MouseCallback(GLFWwindow *window, double xpos, double ypos)
    {
        spdlog::trace("Mouse moved to ({}, {})", xpos, ypos);
        // TODO
    }

    void WindowCloseCallback(GLFWwindow *window)
    {
        spdlog::trace("Window closed");
    }

    // TODO: glfwSetWindowIconifyCallback callback implementieren
    void WindowIconifyCallback(GLFWwindow *window, int iconified) {};

    // ODO: glfwSetFramebufferSizeCallback callback implementieren
    void FramebufferSizeCallback(GLFWwindow *window, int width, int height) {};

    // TODO: callback für drag and drop events

    // TODO: drag and drop

} // namespace

class Window
{
public:
    glm::vec2 getScreenSize() const
    {
        GLFWmonitor *primary{glfwGetPrimaryMonitor()};
        if (primary == nullptr)
        {
            throw std::runtime_error("Failed to get primary monitor");
        }

        const GLFWvidmode *mode{glfwGetVideoMode(primary)};
        if (mode == nullptr)
        {
            throw std::runtime_error("Failed to get video mode");
        }

        return {mode->width, mode->height};
    }

    GLFWwindow *_window;

private:
    void initializeGLAD()
    {
        const int gladVersion = gladLoadGL(glfwGetProcAddress);
        if (gladVersion == 0)
        {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        spdlog::info("GLAD Version: {}.{}", (int)GLAD_VERSION_MAJOR(gladVersion), (int)GLAD_VERSION_MINOR(gladVersion));
    }

    void intitializeOpenGL()
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

public:
    Window()
    {
        if (!glfwInit())
        {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        const auto screenSize{getScreenSize()};
        const auto size{screenSize * 0.75f};

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // GLFWmonitor *primary{glfwGetPrimaryMonitor()};
        _window = glfwCreateWindow(size.x, size.y, "Basti's Window", NULL, NULL);
        if (_window == nullptr)
        {
            throw std::runtime_error("Failed to create window");
        }

        spdlog::info("");

        registerCallbacks();

        // setup OpenGL rendering
        glfwMakeContextCurrent(_window);
        glfwSwapInterval(1);

        initializeGLAD();
        intitializeOpenGL();

        auto extensions = getTextureCompressionExtensions();
        spdlog::info("Extensions: {}", fmt::join(extensions, ", "));
    }

    ~Window()
    {
        glfwDestroyWindow(_window);
        glfwTerminate();
    }


    void close() {
        glfwSetWindowShouldClose(_window, GLFW_TRUE);
    }

protected:
    void registerCallbacks()
    {
        glfwSetWindowUserPointer(_window, this);

        // TODO: add keyboard callback with member function as a callback
        glfwSetKeyCallback(_window, KeyboardCallback);
        glfwSetCursorPosCallback(_window, MouseCallback);
        glfwSetWindowCloseCallback(_window, WindowCloseCallback);

        // TODO: add callbacks for window stet changes
        glfwSetWindowFocusCallback(_window, nullptr);
        glfwSetWindowIconifyCallback(_window, nullptr);
        glfwSetWindowMaximizeCallback(_window, nullptr);
        glfwSetWindowRefreshCallback(_window, nullptr);
        glfwSetWindowPosCallback(_window, nullptr);
        glfwSetWindowSizeCallback(_window, nullptr);
        glfwSetWindowContentScaleCallback(_window, nullptr);

        // Callback für Window-Resizing registrieren
        // glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        // V-Sync aktivieren (0 = off, 1 = on)
    }

public:
    void run()
    {
        glfwMakeContextCurrent(_window);

        bgl::ktx::Loader loader1("/home/bastian/Desktop/bgl-game-demo/assets/idle.ktx2", KTX_TTF_BC3_RGBA);
        bgl::ktx::Loader loader2("/home/bastian/Desktop/bgl-game-demo/assets/walk.ktx2", KTX_TTF_BC3_RGBA);
        loader1.upload();
        loader2.upload();

        while (!glfwWindowShouldClose(_window))
        {
            auto c = ((int)glfwGetTime() % 10) / 10.0f;

            glClearColor(c * 2, c, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glfwSwapBuffers(_window);
            glfwPollEvents();
        }
    }

    void sdsd()
    {
    }
};