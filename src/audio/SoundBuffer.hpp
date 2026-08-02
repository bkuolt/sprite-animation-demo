// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#pragma once

#include <AL/al.h>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>
#include <vector>

namespace bgl::audio
{
/**
 * @brief RAII wrapper for an OpenAL sound buffer.
 */
class SoundBuffer final
{
  public:
    SoundBuffer();
    ~SoundBuffer();

    SoundBuffer(const SoundBuffer &) = delete;
    SoundBuffer &operator=(const SoundBuffer &) = delete;

    SoundBuffer(SoundBuffer &&other) noexcept;
    SoundBuffer &operator=(SoundBuffer &&other) noexcept;

    /**
     * @brief Uploads raw PCM sample data to the OpenAL buffer.
     */
    void loadFromPCM(std::span<const int16_t> samples, int sampleRate, ALenum format = AL_FORMAT_MONO16);

    /**
     * @brief Loads a 16-bit PCM WAV audio file into the buffer.
     */
    bool loadFromFile(const std::filesystem::path &path);

    [[nodiscard]] ALuint getHandle() const noexcept { return m_bufferID; }
    [[nodiscard]] bool isValid() const noexcept { return m_bufferID != 0; }

  private:
    ALuint m_bufferID{0};
};
} // namespace bgl::audio
