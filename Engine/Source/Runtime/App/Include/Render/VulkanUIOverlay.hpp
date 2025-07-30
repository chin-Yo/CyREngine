#pragma once
#define IMGUI_DEFINE_DOCKING
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <iomanip>
#include <vulkan/vulkan.h>
#include "VulkanDebug.h"
#include "VulkanDevice.h"
#include "imgui.h"
#include "Framework/Core/DescriptorPool.hpp"
#include "Framework/Core/DescriptorSetLayout.hpp"

class UIOverlay
{
public:
    VulkanDevice *vulkanDevice{nullptr};

    vks::DescriptorPool descriptorPool;

    UIOverlay(VulkanDevice *device);
    ~UIOverlay();

    void InitImGui(VkInstance instance, VkRenderPass renderPass, VkQueue queue, uint32_t MinImageCount, uint32_t ImageCount);

    void preparePipeline(const VkPipelineCache pipelineCache, const VkRenderPass renderPass, const VkFormat colorFormat, const VkFormat depthFormat);
    void prepareResources();

    bool update();
    void draw();
    void resize(uint32_t width, uint32_t height);

    void freeResources();
    
protected:
    void RenderMenuBar();
    void RenderHierarchy();
    void RenderInspector();
    void RenderProjectBrowser();
    void RenderConsole();
    void RenderViewport();
};
