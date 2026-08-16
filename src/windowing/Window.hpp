// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <entt/entt.hpp>
#include <glm/vec2.hpp>
#include <functional>

namespace bgl::window
{

/**
 * @brief RAII GLFW window wrapper with render-loop, input dispatch, and fullscreen toggle.
 *
 * Non-copyable. Manages a single GLFWwindow and its OpenGL context.
 * Input events are forwarded to an EnTT dispatcher when one is set.
 */
class Window
{
  public:
    using RenderCallback = std::function<void(double time)>;

    Window();
    ~Window();

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

  private:
    void registerCallbacks();
    [[nodiscard]] glm::vec2 getScreenSize() const;

    GLFWwindow    *m_window{nullptr};
    RenderCallback m_renderCallback;

    entt::dispatcher *m_dispatcher{nullptr};

    bool m_isPaused{false};
    bool m_isFullscreen{false};
    int  m_windowedX{0},     m_windowedY{0};
    int  m_windowedWidth{800}, m_windowedHeight{600};
};

} // namespace bgl::window