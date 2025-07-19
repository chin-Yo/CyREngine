#pragma once

#include <memory>

class Engine;

class Editor
{

public:
    Editor();
    virtual ~Editor();

    void Initialize(Engine *RuntimeEngine);
    void Clear();

    void Run();

protected:
    Engine *RuntimeEngine{nullptr};
};
