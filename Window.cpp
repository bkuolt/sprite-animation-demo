#include "Window.hpp"
#include "graphics.hpp"

#include "glad/gl.h"
#include "KHR/khrplatform.h"

#include <GLFW/glfw3.h>
#include <iostream>

#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <fmt/ranges.h>
#include <spdlog/spdlog.h>
#include <fmt/core.h>

#include "ktx.hpp"


namespace
{

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

glm::vec2 Window::getScreenSize() const
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

///////////////////////////////////////////

Window::Window()
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

    bgl::InitializeGLAD();
    bgl::IntitializeOpenGL();
}

Window::~Window()
{
    glfwDestroyWindow(_window);
    glfwTerminate();
}

void Window::close()
{
    glfwSetWindowShouldClose(_window, GLFW_TRUE);
}

void Window::registerCallbacks()
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

void Window::run()
{
    glfwMakeContextCurrent(_window);

    bgl::ktx::Loader loader1("/home/bastian/code/sprite-animation-demo/assets/idle.ktx2", KTX_TTF_BC3_RGBA);
    bgl::ktx::Loader loader2("/home/bastian/code/sprite-animation-demo/assets/walk.ktx2", KTX_TTF_BC3_RGBA);
    loader1.upload();
    loader2.upload();

    while (!glfwWindowShouldClose(_window))
    {
        const auto time = glfwGetTime();
        bgl::Draw(time);

        glfwSwapBuffers(_window);
        glfwPollEvents();
    }
}
