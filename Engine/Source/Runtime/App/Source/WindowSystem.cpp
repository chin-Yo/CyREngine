#include "WindowSystem.hpp"

WindowSystem::~WindowSystem()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void WindowSystem::initialize(WindowCreateInfo create_info)
{
    if (!glfwInit())
    {
        // TODO: LOG_FATAL(__FUNCTION__, "failed to initialize GLFW");
        return;
    }

    m_width = create_info.width;
    m_height = create_info.height;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(create_info.width, create_info.height, create_info.title, nullptr, nullptr);
    if (!m_window)
    {
        // TODO: LOG_FATAL(__FUNCTION__, "failed to create window");
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(m_window);

    // Setup input callbacks
    glfwSetWindowUserPointer(m_window, this);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetCharCallback(m_window, charCallback);
    glfwSetCharModsCallback(m_window, charModsCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);
    glfwSetCursorEnterCallback(m_window, cursorEnterCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
    glfwSetDropCallback(m_window, dropCallback);
    glfwSetWindowSizeCallback(m_window, windowSizeCallback);
    glfwSetWindowCloseCallback(m_window, windowCloseCallback);
    glfwSetWindowIconifyCallback(m_window, windowIconifyCallback);

    glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
}

void WindowSystem::pollEvents() const { glfwPollEvents(); }

bool WindowSystem::shouldClose() const { return glfwWindowShouldClose(m_window); }

void WindowSystem::setTitle(const char *title) { glfwSetWindowTitle(m_window, title); }

GLFWwindow *WindowSystem::getWindow() const { return m_window; }

std::tuple<uint32_t, uint32_t> WindowSystem::getWindowSize() const { return {m_width, m_height}; }

void WindowSystem::setFocusMode(bool mode)
{
    m_is_focus_mode = mode;
    glfwSetInputMode(m_window, GLFW_CURSOR, m_is_focus_mode ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}
