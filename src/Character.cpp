// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "Character.hpp"
#include <ranges>
#include <spdlog/spdlog.h>
#include <string_view>

namespace bgl
{
Character::Character(std::string name) : m_name(std::move(name)) {}

void Character::addAnimation(std::string name, std::shared_ptr<bgl::gl::Texture2DArray> texture, uint32_t frameCount)
{
    m_animations.push_back(AnimationState{std::move(name), std::move(texture), frameCount});
    spdlog::info("Added animation '{}' to character '{}'", m_animations.back().name, m_name);
}

void Character::setAnimationIndex(size_t index)
{
    if (index < m_animations.size())
    {
        m_currentAnimationIndex = index;
    }
}

bool Character::setAnimationByName(std::string_view name)
{
    const auto it = std::ranges::find_if(m_animations,
        [name](const AnimationState &s) { return s.name == name; });

    if (it == m_animations.end()) return false;
    m_currentAnimationIndex = static_cast<size_t>(it - m_animations.begin());
    return true;
}

void Character::nextAnimation()
{
    if (!m_animations.empty())
    {
        m_currentAnimationIndex = (m_currentAnimationIndex + 1) % m_animations.size();
    }
}

void Character::previousAnimation()
{
    if (!m_animations.empty())
    {
        m_currentAnimationIndex = (m_currentAnimationIndex + m_animations.size() - 1) % m_animations.size();
    }
}

const AnimationState *Character::getCurrentAnimation() const
{
    if (m_currentAnimationIndex < m_animations.size())
    {
        return &m_animations[m_currentAnimationIndex];
    }
    return nullptr;
}
} // namespace bgl
