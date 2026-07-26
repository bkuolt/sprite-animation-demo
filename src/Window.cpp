// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#include "Window.hpp"
#include "gfx/Graphics.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

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

        if (key == GLFW_KEY_F && action == GLFW_PRESS)
        {
            if (auto *win = static_cast<Window *>(glfwGetWindowUserPointer(window)))
            {
                win->toggleFullscreen();
            }
        }

        auto *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
        if (win && win->getKeyCallback())
        {
            win->getKeyCallback()(key, key, action, mods);
        }
    }

    void CursorPosCallbackInternal(GLFWwindow *window, double xpos, double ypos)
    {
        spdlog::trace("Mouse moved to ({}, {})", xpos, ypos);

        auto *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
        if (win && win->getCursorPosCallback())
        {
            win->getCursorPosCallback()(xpos, ypos);
        }
    }

    void MouseButtonCallbackInternal(GLFWwindow *window, int button, int action, int mods)
    {
        spdlog::trace("Mouse button {} action {}", button, action);

        auto *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
        if (win && win->getMouseButtonCallback())
        {
            win->getMouseButtonCallback()(button, action, mods);
        }
    }

    void ScrollCallbackInternal(GLFWwindow *window, double xoffset, double yoffset)
    {
        spdlog::trace("Mouse scroll: ({}, {})", xoffset, yoffset);

        auto *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
        if (win && win->getScrollCallback())
        {
            win->getScrollCallback()(xoffset, yoffset);
        }
    }

    void WindowCloseCallback(GLFWwindow * /*window*/)
    {
        spdlog::trace("Window closed");
    }

    void WindowIconifyCallback(GLFWwindow *window, int iconified)
    {
        auto *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
        if (!win) return;
        
        // C++ friend class or public setter? We will just cast and set a property.
        // Actually, we can't easily access _isPaused if it's protected without a friend declaration or public method.
        // Let's declare friend functions in Window.hpp or use a public method.
        // Wait, I can just use glfwGetWindowAttrib(window, GLFW_ICONIFIED) in the run loop instead!
        // The prompt says "Implement GLFW callbacks for minimized, maximized, and occluded window states. Logic: Pause rendering when minimized/occluded to save resources. Add debug logs for state transitions."
        if (iconified)
        {
            spdlog::info("Window minimized");
        }
        else
        {
            spdlog::info("Window restored from minimized state");
        }
    }

    void WindowMaximizeCallback(GLFWwindow *window, int maximized)
    {
        if (maximized)
        {
            spdlog::info("Window maximized");
        }
        else
        {
            spdlog::info("Window restored from maximized state");
        }
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

glm::vec2 Window::getWindowSize() const
{
    int width = 0;
    int height = 0;
    if (_window)
    {
        glfwGetFramebufferSize(_window, &width, &height);
    }
    return {static_cast<float>(width > 0 ? width : 1), static_cast<float>(height > 0 ? height : 1)};
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

void Window::setScrollCallback(ScrollCallback callback)
{
    _scrollCallback = std::move(callback);
}

void Window::setCursorPosCallback(CursorPosCallback callback)
{
    _cursorPosCallback = std::move(callback);
}

void Window::setMouseButtonCallback(MouseButtonCallback callback)
{
    _mouseButtonCallback = std::move(callback);
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
    glfwSetCursorPosCallback(_window, CursorPosCallbackInternal);
    glfwSetMouseButtonCallback(_window, MouseButtonCallbackInternal);
    glfwSetScrollCallback(_window, ScrollCallbackInternal);
    glfwSetWindowCloseCallback(_window, WindowCloseCallback);

    glfwSetWindowFocusCallback(_window, nullptr);
    glfwSetWindowIconifyCallback(_window, WindowIconifyCallback);
    glfwSetWindowMaximizeCallback(_window, WindowMaximizeCallback);
    glfwSetWindowRefreshCallback(_window, nullptr);
    glfwSetWindowPosCallback(_window, nullptr);
    glfwSetWindowSizeCallback(_window, nullptr);
    glfwSetWindowContentScaleCallback(_window, nullptr);
}

void Window::toggleFullscreen()
{
    if (!_window) return;

    if (_isFullscreen)
    {
        // Restore window
        glfwSetWindowMonitor(_window, nullptr, _windowedX, _windowedY, _windowedWidth, _windowedHeight, 0);
        _isFullscreen = false;
        spdlog::info("Switched to windowed mode");
    }
    else
    {
        // Save current window position and size
        glfwGetWindowPos(_window, &_windowedX, &_windowedY);
        glfwGetWindowSize(_window, &_windowedWidth, &_windowedHeight);

        // Switch to fullscreen
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        if (monitor)
        {
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            _isFullscreen = true;
            spdlog::info("Switched to fullscreen mode");
        }
    }
}

void Window::run()
{
    glfwMakeContextCurrent(_window);

    while (!glfwWindowShouldClose(_window))
    {
        // Pause rendering if iconified (minimized) or occluded (if supported, otherwise we just check iconified/width=0)
        int width, height;
        glfwGetFramebufferSize(_window, &width, &height);
        
        bool iconified = glfwGetWindowAttrib(_window, GLFW_ICONIFIED) != 0;
        bool visible = glfwGetWindowAttrib(_window, GLFW_VISIBLE) != 0;

        if (iconified || !visible || width == 0 || height == 0)
        {
            glfwWaitEvents(); // Wait until state changes to save resources
            continue;
        }

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
