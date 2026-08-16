// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

// Must include glad/gl.h BEFORE Qt OpenGL headers to avoid GL header collision
#include <glad/gl.h>

#include <QOpenGLWindow>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QElapsedTimer>
#include <QGuiApplication>

#include <entt/entt.hpp>
#include <glm/vec2.hpp>
#include <functional>
#include <memory>

namespace bgl::window
{

/**
 * @brief Qt QOpenGLWindow wrapper providing OpenGL 4.6 DSA context,
 * continuous real-time rendering loop, input event publishing via EnTT, and window controls.
 */
class Window : public QOpenGLWindow
{
    Q_OBJECT

  public:
    using RenderCallback = std::function<void(double time)>;

    Window();
    ~Window() override;

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    /// Sets the EnTT dispatcher used for input event publishing.
    void setEventDispatcher(entt::dispatcher *dispatcher) noexcept
    {
        m_dispatcher = dispatcher;
    }

    /// Returns the active EnTT dispatcher, or nullptr.
    [[nodiscard]] entt::dispatcher *getEventDispatcher() const noexcept
    {
        return m_dispatcher;
    }

    void setRenderCallback(RenderCallback callback);

    [[nodiscard]] glm::vec2 getWindowSize() const;

    void close();
    void run();
    void run(RenderCallback callback);
    void toggleFullscreen();

  protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

  private:
    RenderCallback m_renderCallback;
    entt::dispatcher *m_dispatcher{nullptr};
    QElapsedTimer m_timer;

    bool m_isFullscreen{false};
};

} // namespace bgl::window