// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include "SoundBuffer.hpp"
#include "SoundSource.hpp"
#include <AL/al.h>
#include <AL/alc.h>
#include <glm/vec3.hpp>

#include <memory>

namespace bgl::audio
{
/**
 * @brief Modern OpenAL Soft audio engine for procedural ambient audio, Jingle Bells melody, and sound playback.
 */
class AudioEngine final
{
  public:
    AudioEngine();
    ~AudioEngine();

    AudioEngine(const AudioEngine &) = delete;
    AudioEngine &operator=(const AudioEngine &) = delete;

    /**
     * @brief Initializes the OpenAL device and context.
     */
    bool init();

    /**
     * @brief Shuts down OpenAL context and device.
     */
    void shutdown();

    /**
     * @brief Updates listener orientation and source states.
     */
    void update(float deltaTime);

    /**
     * @brief Plays background Jingle Bells music loop.
     */
    void playJingleBells();

    /**
     * @brief Plays a procedural jump sound effect.
     */
    void playJumpSound();

    /**
     * @brief Sets 3D position of audio listener (camera).
     */
    void setListenerPosition(const glm::vec3 &pos);

  private:
    ALCdevice *m_device{nullptr};
    ALCcontext *m_context{nullptr};

    SoundBuffer m_jingleBuffer{};
    SoundSource m_jingleSource{};

    SoundBuffer m_jumpBuffer{};
    SoundSource m_jumpSource{};

    bool m_initialized{false};

    void generateJingleBellsBuffer();
    void generateJumpSoundBuffer();
};
} // namespace bgl::audio
