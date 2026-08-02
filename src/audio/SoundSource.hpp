// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "SoundBuffer.hpp"
#include <AL/al.h>
#include <glm/vec3.hpp>

namespace bgl::audio
{
/**
 * @brief RAII wrapper for an OpenAL sound source.
 */
class SoundSource final
{
  public:
    SoundSource();
    ~SoundSource();

    SoundSource(const SoundSource &) = delete;
    SoundSource &operator=(const SoundSource &) = delete;

    SoundSource(SoundSource &&other) noexcept;
    SoundSource &operator=(SoundSource &&other) noexcept;

    void setBuffer(const SoundBuffer &buffer);
    void play();
    void pause();
    void stop();

    void setLooping(bool loop);
    void setPitch(float pitch);
    void setGain(float gain);
    void setPosition(const glm::vec3 &position);

    [[nodiscard]] bool isPlaying() const;
    [[nodiscard]] ALuint getHandle() const noexcept { return m_sourceID; }

  private:
    ALuint m_sourceID{0};
};
} // namespace bgl::audio
