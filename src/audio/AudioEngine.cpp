// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "AudioEngine.hpp"
#include <spdlog/spdlog.h>
#include <cmath>
#include <numbers>
#include <vector>

namespace bgl::audio
{
AudioEngine::AudioEngine()
{
    // Initialise immediately in the constructor so the engine is ready to use
    // without a separate init() call. On failure we log a warning and continue
    // in a disabled state — audio is non-critical.
    if (!init())
    {
        spdlog::warn("[Audio] AudioEngine initialised in disabled state.");
    }
}

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
        if (m_context) alcDestroyContext(m_context);
        alcCloseDevice(m_device);
        m_device  = nullptr;
        m_context = nullptr;
        return false;
    }

    const ALCchar *deviceName = alcGetString(m_device, ALC_DEVICE_SPECIFIER);
    spdlog::info("[Audio] OpenAL device: {}", deviceName ? deviceName : "Default");

    generateJingleBellsBuffer();
    generateJumpSoundBuffer();

    m_initialized = true;
    return true;
}

void AudioEngine::shutdown()
{
    if (!m_initialized) return;

    m_jingleSource.stop();
    m_jumpSource.stop();

    alcMakeContextCurrent(nullptr);
    if (m_context) { alcDestroyContext(m_context); m_context = nullptr; }
    if (m_device)  { alcCloseDevice(m_device);      m_device  = nullptr; }

    m_initialized = false;
    spdlog::info("[Audio] AudioEngine shut down cleanly.");
}

void AudioEngine::update(const float /*deltaTime*/)
{
    if (!m_initialized) return;

    if (!m_jingleSource.isPlaying())
    {
        m_jingleSource.play();
    }
}

void AudioEngine::playJingleBells()
{
    if (!m_initialized) return;
    m_jingleSource.setLooping(true);
    m_jingleSource.setGain(0.4f);
    m_jingleSource.play();
}

void AudioEngine::playJumpSound()
{
    if (!m_initialized) return;
    m_jumpSource.setGain(0.6f);
    m_jumpSource.play();
}

void AudioEngine::setListenerPosition(const glm::vec3 &pos)
{
    if (!m_initialized) return;
    alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
}

void AudioEngine::generateJingleBellsBuffer()
{
    // Note frequencies (Hz).
    static constexpr float E4   = 329.63f;
    static constexpr float G4   = 392.00f;
    static constexpr float C4   = 261.63f;
    static constexpr float D4   = 293.66f;
    static constexpr float F4   = 349.23f;
    static constexpr float REST = 0.0f;

    struct Note { float freq; float duration; };

    // Allocated once per process — the melody data never changes.
    static const std::vector<Note> melody = {
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
        {G4, 0.25f}, {G4, 0.25f}, {F4, 0.25f}, {D4, 0.25f}, {C4, 0.80f}, {REST, 0.40f},
    };

    static constexpr int   kSampleRate = 44100;
    static constexpr float kTau        = 2.0f * std::numbers::pi_v<float>;

    std::vector<int16_t> samples;
    // Reserve upfront to avoid reallocations during the synthesis loop.
    {
        float totalDuration = 0.0f;
        for (const auto &n : melody) totalDuration += n.duration;
        samples.reserve(static_cast<size_t>(kSampleRate * totalDuration) + 128);
    }

    for (const auto &note : melody)
    {
        const size_t noteSamples = static_cast<size_t>(kSampleRate * note.duration);

        if (note.freq == REST)
        {
            samples.resize(samples.size() + noteSamples, 0);
            continue;
        }

        for (size_t i = 0; i < noteSamples; ++i)
        {
            const float t   = static_cast<float>(i) / kSampleRate;
            const float tau = static_cast<float>(i) / static_cast<float>(noteSamples);
            // Bell timbre: fundamental + 2nd + 3rd harmonic with exponential decay.
            const float envelope = std::pow(1.0f - tau, 1.8f);
            const float signal   =
                (std::sin(kTau * note.freq       * t) * 0.6f +
                 std::sin(kTau * note.freq * 2.f * t) * 0.3f +
                 std::sin(kTau * note.freq * 3.f * t) * 0.1f) * envelope;
            samples.push_back(static_cast<int16_t>(signal * 28000.0f));
        }
    }

    m_jingleBuffer.loadFromPCM(samples, kSampleRate);
    m_jingleSource.setBuffer(m_jingleBuffer);
}

void AudioEngine::generateJumpSoundBuffer()
{
    static constexpr int   kSampleRate = 44100;
    static constexpr float kDuration   = 0.22f;
    static constexpr float kTau        = 2.0f * std::numbers::pi_v<float>;

    const size_t totalSamples = static_cast<size_t>(kSampleRate * kDuration);
    std::vector<int16_t> samples(totalSamples);

    float phase = 0.0f;
    for (size_t i = 0; i < totalSamples; ++i)
    {
        const float t        = static_cast<float>(i) / static_cast<float>(totalSamples);
        const float freq     = 220.0f + 580.0f * t;        // Rising chirp.
        phase += kTau * freq / kSampleRate;
        const float envelope = 1.0f - t;
        const float signal   = std::sin(phase) * envelope * 0.35f;
        samples[i] = static_cast<int16_t>(signal * 32767.0f);
    }

    m_jumpBuffer.loadFromPCM(samples, kSampleRate);
    m_jumpSource.setBuffer(m_jumpBuffer);
}
} // namespace bgl::audio
