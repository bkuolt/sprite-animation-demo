// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#pragma once

#include <functional>

struct GLFWwindow;

namespace bgl::window
{

using KeyCallback = std::function<void(int key, int scancode, int action, int mods)>;
using CursorPosCallback = std::function<void(double xpos, double ypos)>;
using MouseButtonCallback = std::function<void(int button, int action, int mods)>;
using ScrollCallback = std::function<void(double xoffset, double yoffset)>;

class InputHandler
{
  public:
    InputHandler() = default;

    void setKeyCallback(KeyCallback callback);
    void setCursorPosCallback(CursorPosCallback callback);
    void setMouseButtonCallback(MouseButtonCallback callback);
    void setScrollCallback(ScrollCallback callback);

    [[nodiscard]] const KeyCallback &getKeyCallback() const { return _keyCallback; }
    [[nodiscard]] const CursorPosCallback &getCursorPosCallback() const { return _cursorPosCallback; }
    [[nodiscard]] const MouseButtonCallback &getMouseButtonCallback() const { return _mouseButtonCallback; }
    [[nodiscard]] const ScrollCallback &getScrollCallback() const { return _scrollCallback; }

  private:
    KeyCallback _keyCallback;
    CursorPosCallback _cursorPosCallback;
    MouseButtonCallback _mouseButtonCallback;
    ScrollCallback _scrollCallback;
};

} // namespace bgl::window
