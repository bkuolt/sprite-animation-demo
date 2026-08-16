// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "gfx/Texture2DArray.hpp"
#include <glm/vec2.hpp>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace bgl
{
/**
 * @brief Represents an animation state containing a Texture2DArray and its total frame count.
 */
struct AnimationState
{
    std::string name;
    std::shared_ptr<bgl::gfx::Texture2DArray> texture;
    uint32_t frameCount{0};
};

/**
 * @brief Manages a character entity, its position, movement physics, and animation states.
 */
class Character
{
  public:
    /**
     * @brief Constructs a new Character with the specified identifier name.
     * @param name The character's name.
     */
    explicit Character(std::string name);

    /**
     * @brief Registers a new animation state with the character.
     * @param name Name of the animation (e.g. "Idle", "Walk", "Jump").
     * @param texture Texture array containing animation frames.
     * @param frameCount Number of frames in the animation layer sequence.
     */
    void addAnimation(std::string name, std::shared_ptr<bgl::gfx::Texture2DArray> texture, uint32_t frameCount);

    /**
     * @brief Sets the active animation index directly.
     * @param index Zero-based index into the registered animations.
     */
    void setAnimationIndex(size_t index);

    /**
     * @brief Selects an animation state matching the given name.
     * @param name The animation state name to activate (compared by value).
     * @return True if found and set; false otherwise.
     */
    bool setAnimationByName(std::string_view name);

    /**
     * @brief Switches to the next available animation sequentially.
     */
    void nextAnimation();

    /**
     * @brief Switches to the previous available animation sequentially.
     */
    void previousAnimation();

    /**
     * @brief Gets the character's identifier name.
     */
    [[nodiscard]] const std::string &getName() const
    {
        return m_name;
    }

    /**
     * @brief Retrieves the currently active animation state.
     * @return Pointer to active AnimationState, or nullptr if no animations exist.
     */
    [[nodiscard]] const AnimationState *getCurrentAnimation() const;

    /**
     * @brief Gets the index of the currently active animation.
     */
    [[nodiscard]] size_t getCurrentAnimationIndex() const
    {
        return m_currentAnimationIndex;
    }

    /**
     * @brief Returns the total number of registered animation states.
     */
    [[nodiscard]] size_t getAnimationCount() const
    {
        return m_animations.size();
    }

    /**
     * @brief Returns the 2D world position of the character.
     */
    [[nodiscard]] const glm::vec2 &getPosition() const
    {
        return m_position;
    }

    /**
     * @brief Sets the 2D world position of the character.
     */
    void setPosition(const glm::vec2 &position)
    {
        m_position = position;
    }

    /**
     * @brief Checks if the character sprite is flipped horizontally.
     */
    [[nodiscard]] bool isFlipped() const
    {
        return m_isFlipped;
    }

    /**
     * @brief Sets whether the character sprite should be horizontally flipped.
     */
    void setFlipped(bool flipped)
    {
        m_isFlipped = flipped;
    }

    /**
     * @brief Gets the current vertical velocity (units per second).
     */
    [[nodiscard]] float getVelocityY() const
    {
        return m_velocityY;
    }

    /**
     * @brief Sets the current vertical velocity.
     */
    void setVelocityY(float velocityY)
    {
        m_velocityY = velocityY;
    }

    /**
     * @brief Returns true if the character is currently airborne (jumping).
     */
    [[nodiscard]] bool isJumping() const
    {
        return m_isJumping;
    }

    /**
     * @brief Sets whether the character is in an airborne jumping state.
     */
    void setJumping(bool jumping)
    {
        m_isJumping = jumping;
    }

  private:
    std::string m_name;                        ///< Character identifier name
    std::vector<AnimationState> m_animations;  ///< Registered animation states
    size_t m_currentAnimationIndex{0};         ///< Currently active animation index
    glm::vec2 m_position{0.0f, 0.0f};          ///< 2D world position
    bool m_isFlipped{false};                   ///< Horizontal flip flag
    float m_velocityY{0.0f};                   ///< Vertical speed (gravity/jump)
    bool m_isJumping{false};                   ///< Airborne status flag
};
} // namespace bgl
