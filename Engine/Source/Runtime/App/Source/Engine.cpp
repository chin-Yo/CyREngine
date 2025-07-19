#include "Engine.hpp"

#include <atomic>
#include <chrono>

const float FPSAlpha = 1.f / 100;
void Engine::CalculateFPS(float DeltaTime)
{
    DeltaTime = std::min(DeltaTime, 0.5f);
    FrameCount++;

    if (FrameCount == 1 || FrameCount % 1000 == 0)
    {
        AverageDuration = DeltaTime;
    }
    else
    {
        AverageDuration = AverageDuration * (1 - FPSAlpha) + DeltaTime * FPSAlpha;
    }
    FPS = static_cast<int>(1.f / AverageDuration);
}

float Engine::CalculateDeltaTime()
{
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - LastTickTimePoint);
    LastTickTimePoint = now;
    return duration.count() / 1000000.0f;
}

void Engine::StartEngine(const std::string &ConfigFilePath)
{

}

void Engine::Initialize()
{
}
