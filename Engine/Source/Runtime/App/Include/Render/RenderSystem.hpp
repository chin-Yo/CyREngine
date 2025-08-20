#pragma once

#include <volk.h>
#include "Framework/Core/VulkanSwapChain.hpp"
#include "VulkanUIOverlay.hpp"
#include <optional>
#include "Framework/Core/RenderPass.hpp"

class RenderSystem
{
private:
    uint32_t destWidth{};
    uint32_t destHeight{};
    void createPipelineCache();
    void createCommandPool();
    void createSynchronizationPrimitives();
    void createSurface();
    void createSwapChain();
    void createCommandBuffers();
    void destroyCommandBuffers();
    void createUI();
    void setupFrameBuffer();
    void UpdateIconityState(bool iconified);

public:
    RenderSystem();
    ~RenderSystem();

    void render();
    bool InitVulkan();
    /** @brief Prepares all Vulkan resources and functions required to run the sample */
    virtual void prepare();
    /** Prepare the next frame for workload submission by acquiring the next swap chain image */
    void prepareFrame();
    /** @brief Presents the current image to the swap chain */
    void submitFrame();
    /** @brief Entry point for the main render loop */
    void renderLoop(float DeltaTime);
    /** @brief (Virtual) Creates the application wide Vulkan instance */
    virtual VkResult CreateInstance();
    void windowResize();

    void buildCommandBuffers();

    UIOverlay *GlobalUI = nullptr;

public:
    bool prepared = false;
    bool IsIconity = false;
    std::string title = "Vulkan Render";
    std::string name = "vulkanRender";
    uint32_t width = 1280;
    uint32_t height = 720;
    uint32_t apiVersion = VK_API_VERSION_1_0;
    /** @brief Example settings that can be changed e.g. by command line arguments */
    struct Settings
    {
        /** @brief Activates validation layers (and message output) when set to true */
        bool validation = false;
        /** @brief Set to true if fullscreen mode has been requested via command line */
        bool fullscreen = false;
        /** @brief Set to true if v-sync will be forced for the swapchain */
        bool vsync = false;
        /** @brief Enable UI overlay */
        bool overlay = true;
    } settings;
    /** @brief Encapsulated physical and logical vulkan device */
    vkb::VulkanDevice *vulkanDevice{};
    bool requiresStencil{false};

    
    ImVec2 m_ViewportSize = { 0.0f, 0.0f };
    struct OffscreenBuffer
    {
        
    }m_OffscreenBuffer; 
    bool m_ViewportResized = false;
    
protected:
    void getEnabledFeatures();

    /** @brief Set of instance extensions to be enabled for this example (must be set in the derived constructor) */
    std::vector<const char *> enabledInstanceExtensions;
    std::vector<std::string> supportedInstanceExtensions;
    /** @brief Set of layer settings to be enabled for this example (must be set in the derived constructor) */
    std::vector<VkLayerSettingEXT> enabledLayerSettings;

    std::optional<vks::RenderPass> UIRenderPass;
    // Frame counter to display fps
    uint32_t frameCounter = 0;
    // Vulkan instance, stores all per-application states
    VkInstance instance{VK_NULL_HANDLE};
    // Physical device (GPU) that Vulkan will use
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    // Stores physical device properties (for e.g. checking device limits)
    VkPhysicalDeviceProperties deviceProperties{};
    // Stores the features available on the selected physical device (for e.g. checking if a feature is available)
    VkPhysicalDeviceFeatures deviceFeatures{};
    // Stores all available memory (type) properties for the physical device
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties{};
    /** @brief Set of physical device features to be enabled for this example (must be set in the derived constructor) */
    VkPhysicalDeviceFeatures enabledFeatures{};
    /** @brief Set of device extensions to be enabled for this example (must be set in the derived constructor) */
    std::vector<const char *> enabledDeviceExtensions;
    /** @brief Optional pNext structure for passing extension structures to device creation */
    void *deviceCreatepNextChain = nullptr;
    /** @brief Logical device, application's view of the physical device (GPU) */
    VkDevice device{VK_NULL_HANDLE};
    VkQueue GraphicsQueue{VK_NULL_HANDLE};
    // Depth buffer format (selected during Vulkan initialization)
    VkFormat depthFormat{VK_FORMAT_UNDEFINED};
    // Command buffer pool
    VkCommandPool cmdPool{VK_NULL_HANDLE};
    // Wraps the swap chain to present images (framebuffers) to the windowing system
    VulkanSwapChain swapChain;
    /** @brief Pipeline stages used to wait at for graphics queue submissions */
    VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // Contains command buffers and semaphores to be presented to the queue
    VkSubmitInfo submitInfo{};
    // Command buffers used for rendering
    std::vector<VkCommandBuffer> drawCmdBuffers;
    // Global render pass for frame buffer writes
    VkRenderPass renderPass{VK_NULL_HANDLE};
    struct
    {
        // Swap chain image presentation
        VkSemaphore presentComplete;
        // Command buffer submission and execution
        VkSemaphore renderComplete;
    } semaphores{};
    std::vector<VkFence> waitFences;
    // List of shader modules created (stored for cleanup)
    std::vector<VkShaderModule> shaderModules;
    // Pipeline cache object
    VkPipelineCache pipelineCache{VK_NULL_HANDLE};
    // List of available frame buffers (same as number of swap chain images)
    std::vector<VkFramebuffer> frameBuffers;

    // Descriptor set pool
    VkDescriptorPool descriptorPool{VK_NULL_HANDLE};

    // Active frame buffer index
    uint32_t currentBuffer = 0;
};