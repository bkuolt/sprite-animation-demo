// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian. All rights reserved.

#pragma once

#include <chrono>
#include <utility>
#include <cmath>
#include <cstdint>

template <typename T>
class Timer
{
public:
    using Duration = std::chrono::duration<T>;

    void start()
    {
        _startTime = std::chrono::high_resolution_clock::now();
        _running = true;
    }

    Duration pause()
    {
        if (_running)
        {
            const auto now = std::chrono::high_resolution_clock::now();
            _elapsedTime += std::chrono::duration_cast<Duration>(now - _startTime);
            _running = false;
        }
        return _elapsedTime;
    }

    void resume()
    {
        if (!_running)
        {
            _startTime = std::chrono::high_resolution_clock::now();
            _running = true;
        }
    }

    Duration stop()
    {
        if (_running)
        {
            const auto now = std::chrono::high_resolution_clock::now();
            _elapsedTime += std::chrono::duration_cast<Duration>(now - _startTime);
            _running = false;
        }
        return _elapsedTime;
    }

    [[nodiscard]] Duration elapsed() const noexcept
    {
        return _elapsedTime;
    }

private:
    std::chrono::high_resolution_clock::time_point _startTime{};
    Duration _elapsedTime{0};
    bool _running{false};
};

class AnimationTimer : public Timer<std::chrono::milliseconds>
{
public:
    AnimationTimer(uint32_t frameCount, float frameDuration)
        : _frameCount(frameCount), _frameDuration(frameDuration)
    {
    }

    AnimationTimer(uint32_t frameCount, unsigned int fps)
        : _frameCount(frameCount), _frameDuration(1.0f / static_cast<float>(fps))
    {
    }

    [[nodiscard]] std::pair<uint32_t, float> frame() const
    {
        const float totalTimeInSeconds = static_cast<float>(elapsed().count()) / 1000.0f;
        const float totalFrames = totalTimeInSeconds / _frameDuration;

        double integralPart = 0.0;
        const float fractionalPart = std::modf(totalFrames, &integralPart);

        const uint32_t frameIndex = static_cast<uint32_t>(integralPart) % _frameCount;

        return {frameIndex, fractionalPart};
    }

private:
    uint32_t _frameCount{0};
    float _frameDuration{0.0f};
};

class Animation
{
public:
    Animation() = default;

    void pause() {}
    void resume() {}
    void stop() {}
    void draw() {}

private:
    AnimationTimer _timer{0, 0.0f};
};