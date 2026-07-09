#include <chrono>
#include <utility>
#include <cmath>

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
            auto now = std::chrono::high_resolution_clock::now();
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
            auto now = std::chrono::high_resolution_clock::now();
            _elapsedTime += std::chrono::duration_cast<Duration>(now - _startTime);
            _running = false;
        }
        return _elapsedTime;
    }

    Duration elapsed() const
    {
        return _elapsedTime;
    }

private:
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
        : _frameCount(frameCount), _frameDuration(1.0f / fps)
    {
    }

    std::pair<uint32_t, float> frame() const
    {
        float tweenFactor;
        float frameIndex;
        const float tweenFactor = std::modf(elapsed().count().count() / _frameDuration, &frameIndex);

        return {std::modf(frameIndex, nullptr), tweenFactor};
    }

private:
    const uint32_t _frameCount;
    const float _frameDuration;
};


class Animation
{
public:
    // TODO: get GL texture handle
    Animation() {
        // get shader
        // get qzad vbo and vao
        // get texture array handle
    }

    // 

    void pause() {
        // TODO
    }

    void resume() {
        // TODO
    }

    void stop() {
        // TODO
    }
    
    void draw()
    {
        // TODO
    }

private:
    AnimationTimer _timer(0, 0.0f);
};