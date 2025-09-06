#pragma once
#define IMGUI_DEFINE_DOCKING
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <iomanip>
#include <volk.h>
#include "Framework/Core/VulkanDevice.hpp"
#include "imgui.h"
#include "Framework/Core/DescriptorPool.hpp"
#include "Framework/Core/DescriptorSetLayout.hpp"
#include "eventpp/callbacklist.h"
#include "Framework/Core/Sampler.hpp"
#include "Framework/Core/VulkanInitializers.hpp"


class UIOverlay
{
public:
    vkb::VulkanDevice *vulkanDevice{nullptr};

    vks::DescriptorPool descriptorPool;

    UIOverlay(vkb::VulkanDevice *device);
    ~UIOverlay();

    void InitImGui(VkInstance instance, VkRenderPass renderPass, VkQueue queue, uint32_t MinImageCount, uint32_t ImageCount);

    void preparePipeline(const VkPipelineCache pipelineCache, const VkRenderPass renderPass, const VkFormat colorFormat, const VkFormat depthFormat);
    void prepareResources();

    bool update();
    void draw(bool Off);
    void resize(uint32_t width, uint32_t height);

    void freeResources();

    eventpp::CallbackList<void(const ImVec2& PortSize)> OnViewportChange;
    ImVec2 m_ViewportSize{0, 0};
    bool m_ViewportResized = false;
    vkb::Sampler *OffScreenSampler = nullptr;
    VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
protected:
    void RenderMenuBar();
    void RenderHierarchy();
    void RenderInspector();
    void RenderProjectBrowser();
    void RenderConsole();
    void RenderViewport(bool off);
};
