// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

namespace bgl::events
{

struct KeyEvent
{
    int key;
    int scancode;
    int action;
    int mods;
};

struct ScrollEvent
{
    double xoffset;
    double yoffset;
};

struct MouseMovedEvent
{
    double xpos;
    double ypos;
};

struct MouseButtonEvent
{
    int button;
    int action;
    int mods;
};

struct WindowResizeEvent
{
    int width;
    int height;
};

} // namespace bgl::events
