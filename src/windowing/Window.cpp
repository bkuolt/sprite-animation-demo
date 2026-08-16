// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "Window.hpp"
#include "../events/Event.hpp"
#include "../gfx/Graphics.hpp"

#include <spdlog/spdlog.h>
#include <stdexcept>

namespace
{
void KeyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    auto *win = static_cast<bgl::window::Window *>(glfwGetWindowUserPointer(window));
    if (!win) return;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        win->close();
        return;
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        win->toggleFullscreen();
    }

    if (auto *d = win->getEventDispatcher())
    {
        d->trigger(bgl::events::KeyEvent{key, scancode, action, mods});
    }
}

void CursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    auto *win = static_cast<bgl::window::Window *>(glfwGetWindowUserPointer(window));
    if (win && win->getEventDispatcher())
    {
        win->getEventDispatcher()->trigger(bgl::events::MouseMovedEvent{xpos, ypos});
    }
}

void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    auto *win = static_cast<bgl::window::Window *>(glfwGetWindowUserPointer(window));
    if (win && win->getEventDispatcher())
    {
        win->getEventDispatcher()->trigger(bgl::events::MouseButtonEvent{button, action, mods});
    }
}

void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    auto *win = static_cast<bgl::window::Window *>(glfwGetWindowUserPointer(window));
    if (win && win->getEventDispatcher())
    {
        win->getEventDispatcher()->trigger(bgl::events::ScrollEvent{xoffset, yoffset});
    }
}

void WindowIconifyCallback(GLFWwindow * /*window*/, int iconified)
{
    spdlog::info(iconified ? "Window minimized" : "Window restored");
}

void WindowMaximizeCallback(GLFWwindow * /*window*/, int maximized)
{
    spdlog::info(maximized ? "Window maximized" : "Window restored from maximized");
}
} // namespace

namespace bgl::window
{

glm::vec2 Window::getScreenSize() const
{
    GLFWmonitor *primary = glfwGetPrimaryMonitor();
    if (!primary) throw std::runtime_error("Failed to get primary monitor");

    const GLFWvidmode *mode = glfwGetVideoMode(primary);
    if (!mode) throw std::runtime_error("Failed to get video mode");

    return {static_cast<float>(mode->width), static_cast<float>(mode->height)};
}

glm::vec2 Window::getWindowSize() const
{
    int width = 0, height = 0;
    if (m_window) glfwGetFramebufferSize(m_window, &width, &height);
    return {static_cast<float>(width > 0 ? width : 1),
            static_cast<float>(height > 0 ? height : 1)};
}

Window::Window()
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    const auto screenSize = getScreenSize();
    const auto size = screenSize * 0.75f;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    m_window = glfwCreateWindow(static_cast<int>(size.x), static_cast<int>(size.y),
                                "BGL Sprite Animation Demo", nullptr, nullptr);
    if (!m_window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    registerCallbacks();

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    bgl::InitializeGLAD();
    bgl::InitializeOpenGL();
}

Window::~Window()
{
    if (m_window)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

void Window::setRenderCallback(RenderCallback callback)
{
    m_renderCallback = std::move(callback);
}

void Window::close()
{
    if (m_window) glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

void Window::registerCallbacks()
{
    glfwSetWindowUserPointer(m_window, this);

    glfwSetKeyCallback(m_window, KeyboardCallback);
    glfwSetCursorPosCallback(m_window, CursorPosCallback);
    glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
    glfwSetScrollCallback(m_window, ScrollCallback);
    glfwSetWindowCloseCallback(m_window, [](GLFWwindow *) { spdlog::trace("Window closed"); });
    glfwSetWindowIconifyCallback(m_window, WindowIconifyCallback);
    glfwSetWindowMaximizeCallback(m_window, WindowMaximizeCallback);
}

void Window::toggleFullscreen()
{
    if (!m_window) return;

    if (m_isFullscreen)
    {
        glfwSetWindowMonitor(m_window, nullptr, m_windowedX, m_windowedY,
                             m_windowedWidth, m_windowedHeight, 0);
        m_isFullscreen = false;
        spdlog::info("Switched to windowed mode");
    }
    else
    {
        glfwGetWindowPos(m_window, &m_windowedX, &m_windowedY);
        glfwGetWindowSize(m_window, &m_windowedWidth, &m_windowedHeight);

        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        if (monitor)
        {
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            m_isFullscreen = true;
            spdlog::info("Switched to fullscreen mode");
        }
    }
}

void Window::run()
{
    glfwMakeContextCurrent(m_window);

    while (!glfwWindowShouldClose(m_window))
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_window, &width, &height);

        const bool iconified = glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) != 0;
        const bool visible   = glfwGetWindowAttrib(m_window, GLFW_VISIBLE) != 0;

        if (iconified || !visible || width == 0 || height == 0)
        {
            glfwWaitEvents();
            continue;
        }

        glViewport(0, 0, width, height);

        if (m_renderCallback) m_renderCallback(glfwGetTime());

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}

void Window::run(RenderCallback callback)
{
    setRenderCallback(std::move(callback));
    run();
}

} // namespace bgl::window
