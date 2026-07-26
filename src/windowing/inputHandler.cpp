// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#include "inputHandler.hpp"
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

namespace bgl::window
{

void InputHandler::setKeyCallback(KeyCallback callback)
{
    _keyCallback = std::move(callback);
}

void InputHandler::setCursorPosCallback(CursorPosCallback callback)
{
    _cursorPosCallback = std::move(callback);
}

void InputHandler::setMouseButtonCallback(MouseButtonCallback callback)
{
    _mouseButtonCallback = std::move(callback);
}

void InputHandler::setScrollCallback(ScrollCallback callback)
{
    _scrollCallback = std::move(callback);
}

} // namespace bgl::window
