#pragma once
#include "Renderer.h"
#include "glm/glm.hpp"

class SkyBox : public IRenderer
{
protected:
    struct SkyUBO
    {
        glm::mat4 view; // 去除了平移的视图矩阵
        glm::mat4 proj; // 投影矩阵
        glm::vec4 sunDirection = glm::vec4(0.0, -0.5, 0.0, 0.0); // 太阳方向（世界空间）
        float time = 0.f; // 时间参数
    } uboSky;

    Buffer SkyUniformBuffer;

    struct AtmosphericParameters
    {
        float MieAnisotropy;
        float RayleighScatteringScalarHeigh;
        float MieScatteringScalarHeight;
        float PlantRadius;
        float OzoneLevelCenterHeight;
        float OzoneLevelwidth;
        float AtmosphereHeight;
    } AtmosphericParam;

    Buffer AtmosphereUniformBuffer;

    
public:
    SkyBox(VulkanDevice* device);
    ~SkyBox() override;


    void CreatePipeline(VkRenderPass renderPass) override;

    void CreatePipelineLayoutAndDescriptors(VkDescriptorSetLayout globalSetLayout) override;

    void prepareSkyUniformBuffers();
};
