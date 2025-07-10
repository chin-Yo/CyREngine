#pragma once

#include "VulkanglTFModel.h"
#include "Body.h"
#include "VulkanTexture.h"


class StaticMesh
{
protected:
    Buffer uniformBufferVS;

public:
    StaticMesh();
    ~StaticMesh();

    struct UBOVS
    {
        glm::mat4 model; // 模型矩阵
        glm::mat4 view; // 视图矩阵
        glm::mat4 proj; // 投影矩阵
        glm::vec4 lightPos;
    } uboVS;

    vks::Texture2D texture;

    VkDescriptorSet descriptorSet;
    void updateUniformBuffers();

    void prepareUniformBuffers(VulkanDevice* vulkanDevice);
    void AllocateSet(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);

    void Tick(float DeltaTime);

    Body* body = nullptr;
    glm::vec3 m_scale = glm::vec3(1.0f);
    vkglTF::Model* model = nullptr;
};
