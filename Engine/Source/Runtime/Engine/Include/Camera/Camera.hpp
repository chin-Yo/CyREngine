#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

// 定义几种可能的摄像机移动方向，用于抽象输入
enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// 默认摄像机参数
const float YAW = -90.0f;       // 偏航角，-90度使得摄像机初始朝向-Z轴
const float PITCH = 0.0f;       // 俯仰角
const float SPEED = 2.5f;       // 移动速度
const float SENSITIVITY = 0.1f; // 鼠标灵敏度
const float FOV = 45.0f;        // 视野 (Field of View)

class Camera
{
public:
    // 摄像机属性
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // 欧拉角
    float Yaw;
    float Pitch;

    // 摄像机选项
    float MovementSpeed;
    float MouseSensitivity;
    float Fov = FOV; // 视野，也常被称为Zoom

    // 构造函数
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = YAW, float pitch = PITCH);

    // 返回视图矩阵
    glm::mat4 GetViewMatrix();

    // 处理从键盘接收到的输入
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);

    // 处理从鼠标移动接收到的输入
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

    // 处理从鼠标滚轮接收到的输入
    void ProcessMouseScroll(float yoffset);

private:
    // 根据摄像机的欧拉角更新 Front, Right, Up 向量
    void updateCameraVectors();
};

#endif