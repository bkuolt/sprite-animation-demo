// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "SoundSource.hpp"

namespace bgl::audio
{
SoundSource::SoundSource()
{
    alGenSources(1, &m_sourceID);
}

SoundSource::~SoundSource()
{
    if (m_sourceID != 0)
    {
        alSourceStop(m_sourceID);
        alDeleteSources(1, &m_sourceID);
        m_sourceID = 0;
    }
}

SoundSource::SoundSource(SoundSource &&other) noexcept
    : m_sourceID{other.m_sourceID}
{
    other.m_sourceID = 0;
}

SoundSource &SoundSource::operator=(SoundSource &&other) noexcept
{
    if (this != &other)
    {
        if (m_sourceID != 0)
        {
            alSourceStop(m_sourceID);
            alDeleteSources(1, &m_sourceID);
        }
        m_sourceID = other.m_sourceID;
        other.m_sourceID = 0;
    }
    return *this;
}

void SoundSource::setBuffer(const SoundBuffer &buffer)
{
    alSourcei(m_sourceID, AL_BUFFER, static_cast<ALint>(buffer.getHandle()));
}

void SoundSource::play()
{
    alSourcePlay(m_sourceID);
}

void SoundSource::pause()
{
    alSourcePause(m_sourceID);
}

void SoundSource::stop()
{
    alSourceStop(m_sourceID);
}

void SoundSource::setLooping(const bool loop)
{
    alSourcei(m_sourceID, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
}

void SoundSource::setPitch(const float pitch)
{
    alSourcef(m_sourceID, AL_PITCH, pitch);
}

void SoundSource::setGain(const float gain)
{
    alSourcef(m_sourceID, AL_GAIN, gain);
}

void SoundSource::setPosition(const glm::vec3 &position)
{
    alSource3f(m_sourceID, AL_POSITION, position.x, position.y, position.z);
}

bool SoundSource::isPlaying() const
{
    ALint state{0};
    alGetSourcei(m_sourceID, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}
} // namespace bgl::audio
