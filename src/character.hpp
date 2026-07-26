// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "gfx/texture2dArray.hpp"
#include <glad/gl.h>
#include <memory>
#include <string>
#include <vector>

namespace bgl
{
struct AnimationState
{
    std::string name;
    std::shared_ptr<bgl::gfx::Texture2DArray> texture;
    uint32_t frameCount{0};
};

class Character
{
  public:
    explicit Character(std::string name);

    void addAnimation(std::string name, std::shared_ptr<bgl::gfx::Texture2DArray> texture, uint32_t frameCount);

    void setAnimationIndex(size_t index);
    void nextAnimation();
    void previousAnimation();

    [[nodiscard]] const std::string &getName() const
    {
        return m_name;
    }
    [[nodiscard]] const AnimationState *getCurrentAnimation() const;
    [[nodiscard]] size_t getCurrentAnimationIndex() const
    {
        return m_currentAnimationIndex;
    }
    [[nodiscard]] size_t getAnimationCount() const
    {
        return m_animations.size();
    }

  private:
    std::string m_name;
    std::vector<AnimationState> m_animations;
    size_t m_currentAnimationIndex{0};
};
} // namespace bgl
