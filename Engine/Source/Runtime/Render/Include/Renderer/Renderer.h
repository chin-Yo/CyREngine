#pragma once
#include <vulkan/vulkan.hpp>

#include "VulkanDevice.h"
#include "VulkanTools.h"


class IRenderer
{
public:
    IRenderer()
    {
    }

    virtual ~IRenderer()
    {
        vkDestroyShaderModule(Device->logicalDevice, shaderModules[0], nullptr);
        vkDestroyShaderModule(Device->logicalDevice, shaderModules[1], nullptr);
        vkDestroyPipeline(Device->logicalDevice, Pipeline, nullptr);
        vkDestroyPipelineLayout(Device->logicalDevice, PipelineLayout, nullptr);
    }

    IRenderer(const IRenderer&) = delete;
    IRenderer& operator=(const IRenderer&) = delete;

    virtual void RenderGameObjects(void*)
    {
    };
    virtual void CreatePipelineLayoutAndDescriptors(VkDescriptorSetLayout globalSetLayout) = 0;
    virtual void CreatePipeline(VkRenderPass renderPass) = 0;
    /** @brief Loads a SPIR-V shader file for the given shader stage */
    VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage);
    //protected:
    VulkanDevice* Device{nullptr};
    // List of shader modules created (stored for cleanup)
    std::vector<VkShaderModule> shaderModules;

    VkPipeline Pipeline;
    VkPipelineLayout PipelineLayout;

    VkPipelineCache pipelineCache{VK_NULL_HANDLE};

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;
    VkDescriptorPool descriptorPool;
};

inline VkPipelineShaderStageCreateInfo IRenderer::loadShader(std::string fileName, VkShaderStageFlagBits stage)
{
    VkPipelineShaderStageCreateInfo shaderStage = {};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = stage;
    shaderStage.module = vks::tools::loadShader(fileName.c_str(), Device->logicalDevice);
    shaderStage.pName = "main";
    assert(shaderStage.module != VK_NULL_HANDLE);
    shaderModules.push_back(shaderStage.module);
    return shaderStage;
}
