#include "Camera/Camera.hpp"

// 带参数的构造函数
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Fov(FOV)
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

// 获取视图矩阵
glm::mat4 Camera::GetViewMatrix()
{
    // glm::lookAt 函数需要一个位置、一个目标点和一个上向量
    // 目标点 = 摄像机位置 + 摄像机朝向
    return glm::lookAt(Position, Position + Front, Up);
}

// 处理键盘输入
void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
    if (direction == UP)
        Position += WorldUp * velocity; // 使用WorldUp实现升降，避免倾斜
    if (direction == DOWN)
        Position -= WorldUp * velocity;
}

// 处理鼠标移动
void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // 确保俯仰角在合理范围内，防止摄像机翻转
    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // 更新 Front, Right 和 Up 向量
    updateCameraVectors();
}

// 处理鼠标滚轮（用于缩放/改变FOV）
void Camera::ProcessMouseScroll(float yoffset)
{
    Fov -= (float)yoffset;
    if (Fov < 1.0f)
        Fov = 1.0f;
    if (Fov > 45.0f) // 你可以根据需要调整最大FOV
        Fov = 45.0f;
}

// 私有方法：更新摄像机方向向量
void Camera::updateCameraVectors()
{
    // 计算新的 Front 向量
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    // 重新计算 Right 和 Up 向量
    // Right 向量是 Front 和 WorldUp 向量的叉积
    Right = glm::normalize(glm::cross(Front, WorldUp));
    // Up 向量是 Right 和 Front 向量的叉积
    Up = glm::normalize(glm::cross(Right, Front));
}