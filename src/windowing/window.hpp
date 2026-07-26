// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "glad/gl.h"
#include "inputHandler.hpp"
#include <GLFW/glfw3.h>
#include <entt/entt.hpp>
#include <glm/vec2.hpp>

namespace bgl::window
{

class Window
{
  public:
    using RenderCallback = std::function<void(double time)>;

    Window();
    ~Window();

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;
    Window(Window &&) noexcept = default;
    Window &operator=(Window &&) noexcept = default;

    [[nodiscard]] InputHandler &getInputHandler() noexcept
    {
        return _inputHandler;
    }
    [[nodiscard]] const InputHandler &getInputHandler() const noexcept
    {
        return _inputHandler;
    }

    void setRenderCallback(RenderCallback callback);
    void setEventDispatcher(entt::dispatcher *dispatcher)
    {
        _inputHandler.setEventDispatcher(dispatcher);
    }

    [[nodiscard]] glm::vec2 getWindowSize() const;

    void close();
    void run();
    void run(RenderCallback callback);
    void toggleFullscreen();

  protected:
    void registerCallbacks();
    [[nodiscard]] glm::vec2 getScreenSize() const;

    GLFWwindow *_window{nullptr};
    RenderCallback _renderCallback;
    InputHandler _inputHandler;

    bool _isPaused{false};
    bool _isFullscreen{false};
    int _windowedX{0}, _windowedY{0};
    int _windowedWidth{800}, _windowedHeight{600};
};

} // namespace bgl::window