// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include <entt/entt.hpp>

struct GLFWwindow;

namespace bgl::window
{

class InputHandler
{
  public:
    InputHandler() = default;

    void setEventDispatcher(entt::dispatcher *dispatcher)
    {
        _dispatcher = dispatcher;
    }

    [[nodiscard]] entt::dispatcher *getEventDispatcher() const
    {
        return _dispatcher;
    }

  private:
    entt::dispatcher *_dispatcher{nullptr};
};

} // namespace bgl::window
