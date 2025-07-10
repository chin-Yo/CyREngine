#pragma once


#include "Renderer.h"


class BlinnPhongRenderer : public IRenderer
{
public:
    BlinnPhongRenderer(VulkanDevice* device);
    ~BlinnPhongRenderer() override;


    void CreatePipeline(VkRenderPass renderPass) override;
    
    void CreatePipelineLayoutAndDescriptors(VkDescriptorSetLayout globalSetLayout) override;
};