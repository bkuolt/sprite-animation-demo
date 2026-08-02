// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "SoundBuffer.hpp"
#include <spdlog/spdlog.h>
#include <fstream>
#include <cstring>

namespace bgl::audio
{
SoundBuffer::SoundBuffer()
{
    alGenBuffers(1, &m_bufferID);
}

SoundBuffer::~SoundBuffer()
{
    if (m_bufferID != 0)
    {
        alDeleteBuffers(1, &m_bufferID);
        m_bufferID = 0;
    }
}

SoundBuffer::SoundBuffer(SoundBuffer &&other) noexcept
    : m_bufferID{other.m_bufferID}
{
    other.m_bufferID = 0;
}

SoundBuffer &SoundBuffer::operator=(SoundBuffer &&other) noexcept
{
    if (this != &other)
    {
        if (m_bufferID != 0)
        {
            alDeleteBuffers(1, &m_bufferID);
        }
        m_bufferID = other.m_bufferID;
        other.m_bufferID = 0;
    }
    return *this;
}

void SoundBuffer::loadFromPCM(std::span<const int16_t> samples, const int sampleRate, const ALenum format)
{
    if (m_bufferID == 0)
    {
        alGenBuffers(1, &m_bufferID);
    }
    alBufferData(m_bufferID, format, samples.data(),
                 static_cast<ALsizei>(samples.size() * sizeof(int16_t)), sampleRate);
}

bool SoundBuffer::loadFromFile(const std::filesystem::path &path)
{
    if (!std::filesystem::exists(path))
    {
        spdlog::error("[Audio] Sound file not found: {}", path.string());
        return false;
    }

    std::ifstream file{path, std::ios::binary};
    if (!file.is_open())
    {
        spdlog::error("[Audio] Cannot open audio file: {}", path.string());
        return false;
    }

    char header[44]{};
    file.read(header, 44);
    if (file.gcount() < 44 || std::memcmp(header, "RIFF", 4) != 0 || std::memcmp(header + 8, "WAVE", 4) != 0)
    {
        spdlog::error("[Audio] File is not a valid RIFF/WAVE file: {}", path.string());
        return false;
    }

    const uint16_t channels = *reinterpret_cast<const uint16_t *>(header + 22);
    const uint32_t sampleRate = *reinterpret_cast<const uint32_t *>(header + 24);
    const uint16_t bitsPerSample = *reinterpret_cast<const uint16_t *>(header + 34);
    const uint32_t dataSize = *reinterpret_cast<const uint32_t *>(header + 40);

    ALenum format{AL_FORMAT_MONO16};
    if (channels == 1 && bitsPerSample == 8) format = AL_FORMAT_MONO8;
    else if (channels == 1 && bitsPerSample == 16) format = AL_FORMAT_MONO16;
    else if (channels == 2 && bitsPerSample == 8) format = AL_FORMAT_STEREO8;
    else if (channels == 2 && bitsPerSample == 16) format = AL_FORMAT_STEREO16;

    std::vector<char> data(dataSize);
    file.read(data.data(), dataSize);

    if (m_bufferID == 0)
    {
        alGenBuffers(1, &m_bufferID);
    }

    alBufferData(m_bufferID, format, data.data(), static_cast<ALsizei>(data.size()), static_cast<ALsizei>(sampleRate));
    spdlog::info("[Audio] Successfully loaded WAV audio file: {}", path.string());
    return true;
}
} // namespace bgl::audio
