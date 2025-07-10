#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (binding = 0) uniform UBO
{
    mat4 model; // 模型矩阵
    mat4 view; // 视图矩阵
    mat4 proj; // 投影矩阵
    vec4 lightPos;
} ubo;

layout (location = 0) out vec2 outUV;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    outUV = inUV;
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPos.xyz, 1.0);
}