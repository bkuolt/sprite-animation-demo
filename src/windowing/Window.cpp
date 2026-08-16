// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "Window.hpp"
#include "../events/Event.hpp"
#include "../gfx/Graphics.hpp"
#include "../gl/GLContext.hpp"

#include <spdlog/spdlog.h>
#include <stdexcept>

namespace bgl::window
{

Window::Window()
{
    QSurfaceFormat format;
    format.setVersion(4, 6);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DebugContext);
    format.setSamples(4);
    setFormat(format);

    setTitle("BGL Sprite Animation Demo (Qt 6 OpenGL 4.6 DSA)");
    resize(1280, 720);

    m_timer.start();
}

Window::~Window() = default;

glm::vec2 Window::getWindowSize() const
{
    return {static_cast<float>(width() > 0 ? width() : 1),
            static_cast<float>(height() > 0 ? height() : 1)};
}

void Window::setRenderCallback(RenderCallback callback)
{
    m_renderCallback = std::move(callback);
}

void Window::close()
{
    QOpenGLWindow::close();
    if (QGuiApplication::instance())
    {
        QGuiApplication::quit();
    }
}

void Window::initializeGL()
{
    bgl::gl::InitializeGLAD();
    bgl::gl::InitializeOpenGL();
}

void Window::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void Window::paintGL()
{
    const double elapsedSeconds = m_timer.elapsed() / 1000.0;
    if (m_renderCallback)
    {
        m_renderCallback(elapsedSeconds);
    }

    // Schedule immediate update for real-time graphics loop
    update();
}

void Window::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        close();
        return;
    }

    if (event->key() == Qt::Key_F)
    {
        toggleFullscreen();
    }

    if (m_dispatcher)
    {
        m_dispatcher->trigger(bgl::events::KeyEvent{event->key(), static_cast<int>(event->nativeScanCode()), 1, 0});
    }
}

void Window::keyReleaseEvent(QKeyEvent *event)
{
    if (m_dispatcher)
    {
        m_dispatcher->trigger(bgl::events::KeyEvent{event->key(), static_cast<int>(event->nativeScanCode()), 0, 0});
    }
}

void Window::mousePressEvent(QMouseEvent *event)
{
    int buttonIndex = 0;
    if (event->button() == Qt::RightButton) buttonIndex = 1;
    else if (event->button() == Qt::MiddleButton) buttonIndex = 2;

    if (m_dispatcher)
    {
        m_dispatcher->trigger(bgl::events::MouseButtonEvent{buttonIndex, 1, 0});
    }
}

void Window::mouseReleaseEvent(QMouseEvent *event)
{
    int buttonIndex = 0;
    if (event->button() == Qt::RightButton) buttonIndex = 1;
    else if (event->button() == Qt::MiddleButton) buttonIndex = 2;

    if (m_dispatcher)
    {
        m_dispatcher->trigger(bgl::events::MouseButtonEvent{buttonIndex, 0, 0});
    }
}

void Window::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dispatcher)
    {
        m_dispatcher->trigger(bgl::events::MouseMovedEvent{event->position().x(), event->position().y()});
    }
}

void Window::wheelEvent(QWheelEvent *event)
{
    if (m_dispatcher)
    {
        const double deltaY = event->angleDelta().y() / 120.0;
        const double deltaX = event->angleDelta().x() / 120.0;
        m_dispatcher->trigger(bgl::events::ScrollEvent{deltaX, deltaY});
    }
}

void Window::toggleFullscreen()
{
    if (m_isFullscreen)
    {
        showNormal();
        m_isFullscreen = false;
        spdlog::info("Switched to windowed mode");
    }
    else
    {
        showFullScreen();
        m_isFullscreen = true;
        spdlog::info("Switched to fullscreen mode");
    }
}

void Window::run()
{
    show();
    if (QGuiApplication::instance())
    {
        QGuiApplication::exec();
    }
}

void Window::run(RenderCallback callback)
{
    setRenderCallback(std::move(callback));
    run();
}

} // namespace bgl::window
