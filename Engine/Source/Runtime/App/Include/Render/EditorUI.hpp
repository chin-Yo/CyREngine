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
#include "Framework/Rendering/Subpass.hpp"


class EditorUIManager
{
public:
    vkb::VulkanDevice& vulkanDevice;

    vks::DescriptorPool descriptorPool;

    EditorUIManager(vkb::VulkanDevice& device);
    ~EditorUIManager();

    void Prepare(VkRenderPass renderPass, VkQueue queue, uint32_t MinImageCount,
                 uint32_t ImageCount);

    void Draw(VkCommandBuffer command_buffer);

    bool update();
    void resize(uint32_t width, uint32_t height);

    void freeResources();

    eventpp::CallbackList<void(const ImVec2& PortSize)> OnViewportChange;
    ImVec2 m_ViewportSize{0, 0};
    bool m_ViewportResized = false;
    vkb::Sampler* OffScreenSampler = nullptr;
    VkRenderPass render_pass{VK_NULL_HANDLE};

protected:
    void RenderMenuBar();
    void RenderHierarchy();
    void RenderInspector();
    void RenderProjectBrowser();
    void RenderConsole();
    void RenderViewport(bool off);
};
