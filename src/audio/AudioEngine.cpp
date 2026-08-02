// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "AudioEngine.hpp"
#include <spdlog/spdlog.h>
#include <cmath>
#include <vector>

namespace bgl::audio
{
AudioEngine::AudioEngine() = default;

AudioEngine::~AudioEngine()
{
    shutdown();
}

bool AudioEngine::init()
{
    if (m_initialized)
    {
        return true;
    }

    m_device = alcOpenDevice(nullptr);
    if (!m_device)
    {
        spdlog::warn("[Audio] Could not open default OpenAL sound device.");
        return false;
    }

    m_context = alcCreateContext(m_device, nullptr);
    if (!m_context || !alcMakeContextCurrent(m_context))
    {
        spdlog::warn("[Audio] Could not create or activate OpenAL context.");
        if (m_context)
        {
            alcDestroyContext(m_context);
        }
        alcCloseDevice(m_device);
        m_device = nullptr;
        m_context = nullptr;
        return false;
    }

    const ALCchar *deviceName{alcGetString(m_device, ALC_DEVICE_SPECIFIER)};
    spdlog::info("[Audio] OpenAL device initialized successfully: {}", deviceName ? deviceName : "Default");

    generateJingleBellsBuffer();
    generateJumpSoundBuffer();

    m_initialized = true;
    return true;
}

void AudioEngine::shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    m_jingleSource.stop();
    m_jumpSource.stop();

    alcMakeContextCurrent(nullptr);
    if (m_context)
    {
        alcDestroyContext(m_context);
        m_context = nullptr;
    }
    if (m_device)
    {
        alcCloseDevice(m_device);
        m_device = nullptr;
    }

    m_initialized = false;
    spdlog::info("[Audio] OpenAL AudioEngine shut down cleanly.");
}

void AudioEngine::update(const float /*deltaTime*/)
{
    if (!m_initialized)
    {
        return;
    }

    if (!m_jingleSource.isPlaying())
    {
        m_jingleSource.play();
    }
}

void AudioEngine::playJingleBells()
{
    if (!m_initialized)
    {
        return;
    }

    m_jingleSource.setLooping(true);
    m_jingleSource.setGain(0.4f);
    m_jingleSource.play();
}

void AudioEngine::playJumpSound()
{
    if (!m_initialized)
    {
        return;
    }

    m_jumpSource.setGain(0.6f);
    m_jumpSource.play();
}

void AudioEngine::setListenerPosition(const glm::vec3 &pos)
{
    if (!m_initialized)
    {
        return;
    }

    alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
}

void AudioEngine::generateJingleBellsBuffer()
{
    // Note frequencies in Hz
    constexpr float E4{329.63f};
    constexpr float G4{392.00f};
    constexpr float C4{261.63f};
    constexpr float D4{293.66f};
    constexpr float F4{349.23f};
    constexpr float REST{0.0f};

    struct Note {
        float freq;
        float duration;
    };

    // Iconic Jingle Bells chorus tune:
    // "Jingle bells, jingle bells, jingle all the way..."
    const std::vector<Note> melody = {
        {E4, 0.25f}, {E4, 0.25f}, {E4, 0.50f},
        {E4, 0.25f}, {E4, 0.25f}, {E4, 0.50f},
        {E4, 0.25f}, {G4, 0.25f}, {C4, 0.35f}, {D4, 0.15f}, {E4, 0.80f},
        {F4, 0.25f}, {F4, 0.25f}, {F4, 0.35f}, {F4, 0.15f},
        {F4, 0.25f}, {E4, 0.25f}, {E4, 0.25f}, {E4, 0.15f}, {E4, 0.15f},
        {E4, 0.25f}, {D4, 0.25f}, {D4, 0.25f}, {E4, 0.25f}, {D4, 0.50f}, {G4, 0.50f},
        {E4, 0.25f}, {E4, 0.25f}, {E4, 0.50f},
        {E4, 0.25f}, {E4, 0.25f}, {E4, 0.50f},
        {E4, 0.25f}, {G4, 0.25f}, {C4, 0.35f}, {D4, 0.15f}, {E4, 0.80f},
        {F4, 0.25f}, {F4, 0.25f}, {F4, 0.35f}, {F4, 0.15f},
        {F4, 0.25f}, {E4, 0.25f}, {E4, 0.25f}, {E4, 0.15f}, {E4, 0.15f},
        {G4, 0.25f}, {G4, 0.25f}, {F4, 0.25f}, {D4, 0.25f}, {C4, 0.80f}, {REST, 0.40f}
    };

    constexpr int sampleRate{44100};
    std::vector<int16_t> samples{};

    for (const auto &note : melody)
    {
        const size_t noteSamples{static_cast<size_t>(sampleRate * note.duration)};
        for (size_t i = 0; i < noteSamples; ++i)
        {
            if (note.freq == REST)
            {
                samples.push_back(0);
                continue;
            }

            const float t{static_cast<float>(i) / sampleRate};
            const float tau{static_cast<float>(i) / noteSamples};
            // Bright bell sound with fundamental + 2nd + 3rd harmonic envelope decay
            const float envelope{std::pow(1.0f - tau, 1.8f)};
            const float sig{
                (std::sin(2.0f * 3.14159f * note.freq * t) * 0.6f +
                 std::sin(2.0f * 3.14159f * note.freq * 2.0f * t) * 0.3f +
                 std::sin(2.0f * 3.14159f * note.freq * 3.0f * t) * 0.1f) * envelope
            };
            samples.push_back(static_cast<int16_t>(sig * 28000.0f));
        }
    }

    m_jingleBuffer.loadFromPCM(samples, sampleRate);
    m_jingleSource.setBuffer(m_jingleBuffer);
}

void AudioEngine::generateJumpSoundBuffer()
{
    constexpr int sampleRate{44100};
    constexpr float duration{0.22f};
    const size_t totalSamples{static_cast<size_t>(sampleRate * duration)};
    std::vector<int16_t> samples(totalSamples);

    float phase{0.0f};
    for (size_t i = 0; i < totalSamples; ++i)
    {
        const float t{static_cast<float>(i) / totalSamples};
        const float freq{220.0f + 580.0f * t};
        phase += 2.0f * 3.14159f * freq / sampleRate;

        const float envelope{1.0f - t};
        const float signal{std::sin(phase) * envelope * 0.35f};

        samples[i] = static_cast<int16_t>(signal * 32767.0f);
    }

    m_jumpBuffer.loadFromPCM(samples, sampleRate);
    m_jumpSource.setBuffer(m_jumpBuffer);
}
} // namespace bgl::audio
