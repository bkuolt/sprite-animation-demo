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

    Window();
    ~Window();

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;
    Window(Window &&) noexcept = default;
    Window &operator=(Window &&) noexcept = default;

    void setRenderCallback(RenderCallback callback);
    void setKeyCallback(KeyCallback callback);
    [[nodiscard]] const KeyCallback &getKeyCallback() const noexcept { return _keyCallback; }

    void close();
    void run();
    void run(RenderCallback callback);

protected:
    void registerCallbacks();
    [[nodiscard]] glm::vec2 getScreenSize() const;

    GLFWwindow *_window{nullptr};
    RenderCallback _renderCallback;
    KeyCallback _keyCallback;
};