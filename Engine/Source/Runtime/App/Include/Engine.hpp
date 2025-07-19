#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <string>
#include <unordered_set>

struct EngineConfig
{
    int MaxFPS = 60;
    bool EnableVSync = true;
    // 可添加其他配置参数...
};

class Engine
{
    static const float FPSAlpha;
    friend class Editor;
    
public:
    void StartEngine(const std::string &ConfigFilePath);
    void ShutdownEngine();

    void Initialize();
    void Clear();

    bool IsQuit() const { return IsQuit; }
    void Run();
    bool TickOneFrame(float DeltaTime);

    int GetFPS() const { return FPS; }
    void SetMaxFPS(int fps) { MaxFPS = fps; }
    std::string GetEngineStatus() const;

protected:
    void LogicalTick(float DeltaTime);
    bool RendererTick(float DeltaTime);

    void CalculateFPS(float DeltaTime);
    float CalculateDeltaTime();

protected:
    std::atomic<bool> IsQuit{false};
    int MaxFPS = 60;

    std::chrono::steady_clock::time_point LastTickTimePoint{std::chrono::steady_clock::now()};

    float AverageDuration = 0.f;
    int FrameCount = 0;
    int FPS = 0;
    EngineConfig mConfig;
};