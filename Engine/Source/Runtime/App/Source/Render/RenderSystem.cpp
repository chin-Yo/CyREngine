#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#include "Render/RenderSystem.hpp"
#include <iostream>
#include "GlobalContext.hpp"
#include <backends/imgui_impl_vulkan.h>

#include "Framework/Core/Image.hpp"
#include "Framework/Misc/SpirvReflection.hpp"
#include "Misc/FileLoader.hpp"
#include "Framework/Core/VulkanTools.hpp"
#include "Framework/Core/VulkanInitializers.hpp"
#include "Framework/Core/VulkanDebug.hpp"
#include "Framework/Core/VulkanDevice.hpp"
#include "Misc/Paths.hpp"

using namespace spv;
using namespace SPIRV_CROSS_NAMESPACE;
using namespace std;

RenderSystem::RenderSystem()
{
    settings.validation = true;
    // SRS - Enable VK_KHR_get_physical_device_properties2 to retrieve device driver information for display
    enabledInstanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
}

RenderSystem::~RenderSystem()
{
}

void RenderSystem::createPipelineCache()
{
}
void RenderSystem::createCommandPool()
{
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = swapChain.queueNodeIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &cmdPool))
}
void RenderSystem::createSynchronizationPrimitives()
{
    // Wait fences to sync command buffer access
    VkFenceCreateInfo fenceCreateInfo = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    waitFences.resize(drawCmdBuffers.size());
    for (auto &fence : waitFences)
    {
        VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));
    }
}
void RenderSystem::createSurface()
{
    swapChain.initSurface(GRuntimeGlobalContext.windowSystem->getWindow());
}
void RenderSystem::createSwapChain()
{
    std::tie(width, height) = GRuntimeGlobalContext.windowSystem->getWindowSize();
    swapChain.create(width, height, settings.vsync, settings.fullscreen);
}
void RenderSystem::createCommandBuffers()
{
    // Create one command buffer for each swap chain image
    drawCmdBuffers.resize(swapChain.images.size());
    VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(
        cmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<uint32_t>(drawCmdBuffers.size()));
    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, drawCmdBuffers.data()));
}
void RenderSystem::destroyCommandBuffers()
{
    vkFreeCommandBuffers(device, cmdPool, static_cast<uint32_t>(drawCmdBuffers.size()), drawCmdBuffers.data());
}

void RenderSystem::createUI()
{
    GlobalUI = new UIOverlay(vulkanDevice);

    UIRenderPass.emplace(vks::RenderPassBuilder(vulkanDevice->logicalDevice)
                             .addAttachment(swapChain.colorFormat, VK_SAMPLE_COUNT_1_BIT,
                                            VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
                                            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
                             .addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS, {0})
                             .addDependency(VK_SUBPASS_EXTERNAL, 0,
                                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
                             .build());

    GlobalUI->InitImGui(instance, UIRenderPass.value(), GraphicsQueue, swapChain.images.size(), swapChain.images.size());
}

void RenderSystem::setupFrameBuffer()
{
    // Create frame buffers for every swap chain image
    frameBuffers.resize(swapChain.images.size());
    for (uint32_t i = 0; i < frameBuffers.size(); i++)
    {
        const VkImageView attachments[1] = {
            swapChain.imageViews[i]};
        VkFramebufferCreateInfo frameBufferCreateInfo{};
        frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferCreateInfo.renderPass = UIRenderPass.value().get();
        frameBufferCreateInfo.attachmentCount = 1;
        frameBufferCreateInfo.pAttachments = attachments;
        frameBufferCreateInfo.width = width;
        frameBufferCreateInfo.height = height;
        frameBufferCreateInfo.layers = 1;
        VK_CHECK_RESULT(vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &frameBuffers[i]));
    }
}

void RenderSystem::UpdateIconityState(bool iconified)
{
    IsIconity = iconified;
}

void RenderSystem::getEnabledFeatures()
{
    if (deviceFeatures.samplerAnisotropy)
    {
        enabledFeatures.samplerAnisotropy = 1;
    }
}
void RenderSystem::render()
{
}
bool RenderSystem::InitVulkan()
{
    VK_CHECK_RESULT(volkInitialize());
    
    // Create the instance
    VkResult result = CreateInstance();
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan instance");
        return false;
    }
    volkLoadInstance(instance);
    // If requested, we enable the default validation layers for debugging
    if (settings.validation)
    {
        vks::debug::setupDebugging(instance);
    }
    // Physical device
    uint32_t gpuCount = 0;
    // Get number of available physical devices
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr));
    if (gpuCount == 0)
    {
        vks::tools::exitFatal("No device with Vulkan support found", -1);
        return false;
    }
    // Enumerate devices
    std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
    result = vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data());
    if (result != VK_SUCCESS)
    {
        vks::tools::exitFatal("Could not enumerate physical devices : \n" + vks::tools::errorString(result), result);
        return false;
    }

    // GPU selection

    // Select physical device to be used for the Vulkan example
    // Defaults to the first device unless specified by command line
    uint32_t selectedDevice = 0;

    physicalDevice = physicalDevices[selectedDevice];

    // Store properties (including limits), features and memory properties of the physical device (so that examples can check against them)
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

    // Derived examples can override this to set actual features (based on above readings) to enable for logical device creation
    getEnabledFeatures();

    // Vulkan device creation
    // This is handled by a separate class that gets a logical device representation
    // and encapsulates functions related to a device
    vulkanDevice = new vkb::VulkanDevice(physicalDevice);

    result = vulkanDevice->createLogicalDevice(enabledFeatures, enabledDeviceExtensions, deviceCreatepNextChain);
    if (result != VK_SUCCESS)
    {
        vks::tools::exitFatal("Could not create Vulkan device: \n" + vks::tools::errorString(result), result);
        return false;
    }
    vulkanDevice->instance = instance;
    vkb::InitVma(*vulkanDevice);
    device = vulkanDevice->logicalDevice;
    volkLoadDevice(device);
    // Get a graphics queue from the device
    vkGetDeviceQueue(device, vulkanDevice->queueFamilyIndices.graphics, 0, &GraphicsQueue);

    // Find a suitable depth and/or stencil format
    VkBool32 validFormat{false};
    // Samples that make use of stencil will require a depth + stencil format, so we select from a different list
    if (requiresStencil)
    {
        validFormat = vks::tools::getSupportedDepthStencilFormat(physicalDevice, &depthFormat);
    }
    else
    {
        validFormat = vks::tools::getSupportedDepthFormat(physicalDevice, &depthFormat);
    }
    assert(validFormat);

    swapChain.setContext(instance, physicalDevice, device);

    // Create synchronization objects
    VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
    // Create a semaphore used to synchronize image presentation
    // Ensures that the image is displayed before we start submitting new commands to the queue
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.presentComplete));
    // Create a semaphore used to synchronize command submission
    // Ensures that the image is not presented until all commands have been submitted and executed
    VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.renderComplete));

    submitInfo = vks::initializers::submitInfo();
    submitInfo.pWaitDstStageMask = &submitPipelineStages;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphores.presentComplete;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &semaphores.renderComplete;

    return true;
}

void RenderSystem::prepare()
{
    GRuntimeGlobalContext.windowSystem->registerOnWindowIconifyFunc(
        [this](bool minimized)
        { this->UpdateIconityState(minimized); });
    createSurface();
    createCommandPool();
    createSwapChain();
    createCommandBuffers();
    createSynchronizationPrimitives();
    createPipelineCache();
    createUI();
    setupFrameBuffer();
    
    prepared = true;

    auto spvfrag = FileLoader::ReadShaderBinaryU32(Paths::GetShaderFullPath("Default/BlinnPhong/BlinnPhong.frag.spv"));
    auto spvvert = FileLoader::ReadShaderBinaryU32(Paths::GetShaderFullPath("Default/BlinnPhong/BlinnPhong.vert.spv"));

    //Spirv::SpirvReflection::reflect_shader(Paths::GetShaderFullPath("Default/BlinnPhong/BlinnPhong.frag.spv"));
    /*vkb::SPIRVReflection spirvReflection;
    std::vector<vkb::ShaderResource> vertresources;
    vkb::ShaderVariant vertvariant;
    spirvReflection.reflect_shader_resources(VK_SHADER_STAGE_VERTEX_BIT, spvvert, vertresources, vertvariant);
    std::vector<vkb::ShaderResource> fragresources;
    vkb::ShaderVariant fragvariant;
    spirvReflection.reflect_shader_resources(VK_SHADER_STAGE_VERTEX_BIT, spvfrag, fragresources, fragvariant);

    vkb::Image image{*vulkanDevice,vkb::ImageBuilder{ 512,512,1}.with_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)};*/
    vkb::ShaderVariant vertvariant;
    vkb::ShaderModule SMVert{*vulkanDevice,VK_SHADER_STAGE_VERTEX_BIT,Paths::GetShaderFullPath("Default/BlinnPhong/BlinnPhong.vert.spv")
        ,"main",vertvariant};

    vkb::ShaderVariant fragvariant;
    vkb::ShaderModule SMFrag{*vulkanDevice,VK_SHADER_STAGE_FRAGMENT_BIT,Paths::GetShaderFullPath("Default/BlinnPhong/BlinnPhong.frag.spv")
        ,"main",fragvariant};

    std::vector<vkb::ShaderModule*> shaders = {&SMVert,&SMFrag};
    
    vkb::PipelineLayout layout{*vulkanDevice,shaders};
}

void RenderSystem::prepareFrame()
{
    // Acquire the next image from the swap chain
    VkResult result = swapChain.acquireNextImage(semaphores.presentComplete, currentBuffer);
    // Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE)
    // SRS - If no longer optimal (VK_SUBOPTIMAL_KHR), wait until submitFrame() in case number of swapchain images will change on resize
    if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR))
    {
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            windowResize();
        }
        return;
    }
    else
    {
        VK_CHECK_RESULT(result)
    }
}

void RenderSystem::submitFrame()
{
    VkResult result = swapChain.queuePresent(GraphicsQueue, currentBuffer, semaphores.renderComplete);
    // Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE) or no longer optimal for presentation (SUBOPTIMAL)
    if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR))
    {
        windowResize();
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            return;
        }
    }
    else
    {
        VK_CHECK_RESULT(result)
    }
    VK_CHECK_RESULT(vkQueueWaitIdle(GraphicsQueue))
}

void RenderSystem::buildCommandBuffers()
{
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

    VkClearValue clearValues[1];
    clearValues[0].color = {{0.2f, 0.2f, 0.2f, 1.0f}};

    VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
    renderPassBeginInfo.renderPass = UIRenderPass.value().get();
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = width;
    renderPassBeginInfo.renderArea.extent.height = height;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = clearValues;
    GlobalUI->draw();
    for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
    {
        // Set target frame buffer
        renderPassBeginInfo.framebuffer = frameBuffers[i];

        VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

        vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), drawCmdBuffers[i]);

        vkCmdEndRenderPass(drawCmdBuffers[i]);

        VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
    }
    // Update and Render additional Platform Windows
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}
void RenderSystem::windowResize()
{
    if (!prepared)
    {
        return;
    }
    prepared = false;
    // Ensure all operations on the device have been finished before destroying resources
    vkDeviceWaitIdle(device);

    createSwapChain();

    for (auto &frameBuffer : frameBuffers)
    {
        vkDestroyFramebuffer(device, frameBuffer, nullptr);
    }
    setupFrameBuffer();

    // Command buffers need to be recreated as they may store
    // references to the recreated frame buffer
    destroyCommandBuffers();
    createCommandBuffers();
    buildCommandBuffers();
    // SRS - Recreate fences in case number of swapchain images has changed on resize
    for (auto &fence : waitFences)
    {
        vkDestroyFence(device, fence, nullptr);
    }
    createSynchronizationPrimitives();

    vkDeviceWaitIdle(device);

    prepared = true;
}
void RenderSystem::renderLoop(float DeltaTime)
{
    if (!prepared || IsIconity)
    {
        return;
    }
    prepareFrame();
    buildCommandBuffers();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
    VK_CHECK_RESULT(vkQueueSubmit(GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE))
    submitFrame();
}

/**
 * @brief Create Vulkan instance
 *
 * This function is responsible for initializing the Vulkan instance, including setting the required extensions,
 * validation layers, and debugging features. It will check the supported extensions and validation layers
 * of the system, and enable the corresponding functions based on the settings.
 *
 * @return VkResult - The result code of the Vulkan operation. VK_SUCCESS indicates successful creation of the instance.
 */
VkResult RenderSystem::CreateInstance()
{
    std::vector<const char *> instanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME};

    // Enable surface extensions depending on os
    instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

    // Get extensions supported by the instance and store for later use
    uint32_t extCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
    if (extCount > 0)
    {
        std::vector<VkExtensionProperties> extensions(extCount);
        if (vkEnumerateInstanceExtensionProperties(nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
        {
            for (VkExtensionProperties &extension : extensions)
            {
                supportedInstanceExtensions.push_back(extension.extensionName);
            }
        }
    }

    // Enabled requested instance extensions
    if (!enabledInstanceExtensions.empty())
    {
        for (const char *enabledExtension : enabledInstanceExtensions)
        {
            // Output message if requested extension is not available
            if (std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(), enabledExtension) ==
                supportedInstanceExtensions.end())
            {
                std::cerr << "Enabled instance extension \"" << enabledExtension << "\" is not present at instance level\n";
            }
            instanceExtensions.push_back(enabledExtension);
        }
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = name.c_str();
    appInfo.pEngineName = name.c_str();
    appInfo.apiVersion = apiVersion;

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI{};
    if (settings.validation)
    {
        vks::debug::setupDebugingMessengerCreateInfo(debugUtilsMessengerCI);
        debugUtilsMessengerCI.pNext = instanceCreateInfo.pNext;
        instanceCreateInfo.pNext = &debugUtilsMessengerCI;
    }

    // Enable the debug utils extension if available (e.g. when debugging tools are present)
    if (settings.validation || std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(),
                                         VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != supportedInstanceExtensions.end())
    {
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    if (!instanceExtensions.empty())
    {
        instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
    }

    // The VK_LAYER_KHRONOS_validation contains all current validation functionality.
    // Note that on Android this layer requires at least NDK r20
    const char *validationLayerName = "VK_LAYER_KHRONOS_validation";
    if (settings.validation)
    {
        // Check if this layer is available at instance level
        uint32_t instanceLayerCount;
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
        std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
        bool validationLayerPresent = false;
        for (VkLayerProperties &layer : instanceLayerProperties)
        {
            if (strcmp(layer.layerName, validationLayerName) == 0)
            {
                validationLayerPresent = true;
                break;
            }
        }
        if (validationLayerPresent)
        {
            instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
            instanceCreateInfo.enabledLayerCount = 1;
        }
        else
        {
            std::cerr << "Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled";
        }
    }

    // If layer settings are defined, then activate the sample's required layer settings during instance creation.
    // Layer settings are typically used to activate specific features of a layer, such as the Validation Layer's
    // printf feature, or to configure specific capabilities of drivers such as MoltenVK on macOS and/or iOS.
    VkLayerSettingsCreateInfoEXT layerSettingsCreateInfo{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT};
    if (enabledLayerSettings.size() > 0)
    {
        layerSettingsCreateInfo.settingCount = static_cast<uint32_t>(enabledLayerSettings.size());
        layerSettingsCreateInfo.pSettings = enabledLayerSettings.data();
        layerSettingsCreateInfo.pNext = instanceCreateInfo.pNext;
        instanceCreateInfo.pNext = &layerSettingsCreateInfo;
    }

    VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

    // If the debug utils extension is present we set up debug functions, so samples can label objects for debugging
    if (std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(),
                  VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != supportedInstanceExtensions.end())
    {
        vks::debugutils::setup(instance);
    }

    return result;
}