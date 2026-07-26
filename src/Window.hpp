// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#pragma once

#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <functional>

class Window
{
public:
    using RenderCallback = std::function<void(double time)>;
    using KeyCallback = std::function<void(int key, int scancode, int action, int mods)>;
    using ScrollCallback = std::function<void(double xoffset, double yoffset)>;
    using CursorPosCallback = std::function<void(double xpos, double ypos)>;
    using MouseButtonCallback = std::function<void(int button, int action, int mods)>;

    Window();
    ~Window();

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;
    Window(Window &&) noexcept = default;
    Window &operator=(Window &&) noexcept = default;

    void setRenderCallback(RenderCallback callback);
    void setKeyCallback(KeyCallback callback);
    void setScrollCallback(ScrollCallback callback);
    void setCursorPosCallback(CursorPosCallback callback);
    void setMouseButtonCallback(MouseButtonCallback callback);

    [[nodiscard]] const KeyCallback &getKeyCallback() const noexcept { return _keyCallback; }
    [[nodiscard]] const ScrollCallback &getScrollCallback() const noexcept { return _scrollCallback; }
    [[nodiscard]] const CursorPosCallback &getCursorPosCallback() const noexcept { return _cursorPosCallback; }
    [[nodiscard]] const MouseButtonCallback &getMouseButtonCallback() const noexcept { return _mouseButtonCallback; }

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
    KeyCallback _keyCallback;
    ScrollCallback _scrollCallback;
    CursorPosCallback _cursorPosCallback;
    MouseButtonCallback _mouseButtonCallback;

    bool _isPaused{false};
    bool _isFullscreen{false};
    int _windowedX{0}, _windowedY{0};
    int _windowedWidth{800}, _windowedHeight{600};
};