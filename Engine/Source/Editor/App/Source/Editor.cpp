#include "App/Include/Editor.hpp"
#include <assert.h>
#include "App/Include/Engine.hpp"
#include "Engine.hpp"

Editor::Editor()
{
}

Editor::~Editor()
{
}

void Editor::Initialize(Engine *RuntimeEngine)
{
    assert(RuntimeEngine);
    this->RuntimeEngine = RuntimeEngine;
}

void Editor::Clear()
{
}

void Editor::Run()
{
    assert(RuntimeEngine);

    float delta_time;
    while (true)
    {
        delta_time = RuntimeEngine->CalculateDeltaTime();

        if (!RuntimeEngine->TickOneFrame(delta_time))
            return;
    }
}

bool Engine::TickOneFrame(float DeltaTime)
{
    LogicalTick(DeltaTime);

    RendererTick(DeltaTime);
    return false;
}
