#include "App/Include/Editor.hpp"
#include <assert.h>
#include "App/Include/Engine.hpp"

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
        RuntimeEngine->LimitFPS(delta_time);

        if (!RuntimeEngine->TickOneFrame(delta_time))
            return;
    }
}
