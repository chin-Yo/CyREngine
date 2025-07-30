#include "GlobalContext.hpp"
#include "WindowSystem.hpp"
#include "Render/RenderSystem.hpp"

RuntimeGlobalContext GRuntimeGlobalContext;

void RuntimeGlobalContext::startSystems(const std::string &config_file_path)
{
    windowSystem = std::make_shared<WindowSystem>();
    WindowCreateInfo create_info;
    windowSystem->initialize(create_info);

    m_render_system = std::make_shared<RenderSystem>();
    m_render_system->InitVulkan();
    m_render_system->prepare();
}

void RuntimeGlobalContext::shutdownSystems()
{
}