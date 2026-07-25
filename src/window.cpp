// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#include "window.hpp"
#include "gfx/graphics.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

int currentAnimation = 0;

namespace
{
    void KeyboardCallback(GLFWwindow *window, int key, int /*scancode*/, int action, int mods)
    {
        spdlog::trace("Key {} pressed", key);

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            return;
        }

        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        {
            currentAnimation++;
        }

        auto *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
        if (win && win->getKeyCallback())
        {
            win->getKeyCallback()(key, key, action, mods);
        }
    }

    void MouseCallback(GLFWwindow * /*window*/, double xpos, double ypos)
    {
        spdlog::trace("Mouse moved to ({}, {})", xpos, ypos);
    }

    void WindowCloseCallback(GLFWwindow * /*window*/)
    {
        spdlog::trace("Window closed");
    }
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

    return {static_cast<float>(mode->width), static_cast<float>(mode->height)};
}

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

    _window = glfwCreateWindow(static_cast<int>(size.x), static_cast<int>(size.y), "Basti's Window", nullptr, nullptr);
    if (_window == nullptr)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create window");
    }

    registerCallbacks();

    // Setup OpenGL rendering context
    glfwMakeContextCurrent(_window);
    glfwSwapInterval(1);

    bgl::InitializeGLAD();
    bgl::IntitializeOpenGL();
}

Window::~Window()
{
    if (_window)
    {
        glfwDestroyWindow(_window);
        _window = nullptr;
    }
    glfwTerminate();
}

void Window::setRenderCallback(RenderCallback callback)
{
    _renderCallback = std::move(callback);
}

void Window::setKeyCallback(KeyCallback callback)
{
    _keyCallback = std::move(callback);
}

void Window::close()
{
    if (_window)
    {
        glfwSetWindowShouldClose(_window, GLFW_TRUE);
    }
}

void Window::registerCallbacks()
{
    glfwSetWindowUserPointer(_window, this);

    glfwSetKeyCallback(_window, KeyboardCallback);
    glfwSetCursorPosCallback(_window, MouseCallback);
    glfwSetWindowCloseCallback(_window, WindowCloseCallback);

    glfwSetWindowFocusCallback(_window, nullptr);
    glfwSetWindowIconifyCallback(_window, nullptr);
    glfwSetWindowMaximizeCallback(_window, nullptr);
    glfwSetWindowRefreshCallback(_window, nullptr);
    glfwSetWindowPosCallback(_window, nullptr);
    glfwSetWindowSizeCallback(_window, nullptr);
    glfwSetWindowContentScaleCallback(_window, nullptr);
}

void Window::run()
{
    glfwMakeContextCurrent(_window);

    while (!glfwWindowShouldClose(_window))
    {
        const auto time = glfwGetTime();
        if (_renderCallback)
        {
            _renderCallback(time);
        }

        glfwSwapBuffers(_window);
        glfwPollEvents();
    }
}

void Window::run(RenderCallback callback)
{
    setRenderCallback(std::move(callback));
    run();
}
