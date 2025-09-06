#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#include "Render/RenderSystem.hpp"
#include <iostream>
#include "GlobalContext.hpp"
#include <backends/imgui_impl_vulkan.h>

#include "Framework/Core/Image.hpp"
#include "Framework/Core/Buffer.hpp"
#include "Framework/Core/Sampler.hpp"
#include "Framework/Misc/SpirvReflection.hpp"
#include "Misc/FileLoader.hpp"
#include "Framework/Core/VulkanTools.hpp"
#include "Framework/Core/VulkanInitializers.hpp"
#include "Framework/Core/VulkanDebug.hpp"
#include "Framework/Core/VulkanDevice.hpp"
#include "Framework/Core/VulkanglTFModel.hpp"
#include "Framework/Rendering/RenderTarget.hpp"
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
    OffScreenDrawCmdBuffers.resize(swapChain.images.size());
    VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(
        cmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<uint32_t>(drawCmdBuffers.size()));
    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, drawCmdBuffers.data()));
    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, OffScreenDrawCmdBuffers.data()));
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
    GlobalUI->OnViewportChange.append([this](const ImVec2& Size)
    {
        this->ViewportResize(Size);
    });
}

void RenderSystem::ViewportResize(const ImVec2& Size)
{
    OffScreenSize = Size;
    if (OffScreenSize.x < 256 && OffScreenSize.y < 256)
    {
        return;
    }
    delete OffScreenRT;
    delete OffScreenFB;
    OffScreenRT = nullptr;
    OffScreenFB = nullptr;
    
    vkDeviceWaitIdle(vulkanDevice->logicalDevice);
    std::vector<vkb::Image> images;
    images.emplace_back(*vulkanDevice,vkb::ImageBuilder{ static_cast<unsigned int>(OffScreenSize.x),static_cast<unsigned int>(OffScreenSize.y),1}
        .with_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT)
        .with_format(VK_FORMAT_R8G8B8A8_UNORM));
    images.emplace_back(*vulkanDevice,vkb::ImageBuilder{ static_cast<unsigned int>(OffScreenSize.x),static_cast<unsigned int>(OffScreenSize.y),1}
        .with_usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        .with_format(VK_FORMAT_D32_SFLOAT));
    
    OffScreenRT = new vkb::RenderTarget{std::move(images)};
    OffScreenFB = new vkb::Framebuffer{*vulkanDevice,*OffScreenRT,*render_pass};
    
    if (GlobalUI->m_DescriptorSet != VK_NULL_HANDLE)
    {
        ImGui_ImplVulkan_RemoveTexture(GlobalUI->m_DescriptorSet);
        GlobalUI->m_DescriptorSet = VK_NULL_HANDLE;
    }
    GlobalUI->m_DescriptorSet = ImGui_ImplVulkan_AddTexture(GlobalUI->OffScreenSampler->GetHandle(),OffScreenRT->get_views()[0].GetHandle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    OffScreenResourcesReady = true;
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

void RenderSystem::UpdateUnifrom()
{
    Scene.view = camera.GetViewMatrix();
    Scene.projection = glm::perspective(glm::radians(camera.Fov), (float)OffScreenSize.x / (float)OffScreenSize.y, 0.1f, 100.0f);
    Scene.cameraPos = camera.Position;
    uboScene->update(&Scene, sizeof(UboScene));
    uboLight->update(&Light, sizeof(UboLight));
    uboMaterial->update(&Material, sizeof(UboMaterial));
    
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
    const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;

    auto spvfrag = FileLoader::ReadShaderBinaryU32(Paths::GetShaderFullPath("Default/BlinnPhong/BlinnPhong.frag.spv"));
    auto spvvert = FileLoader::ReadShaderBinaryU32(Paths::GetShaderFullPath("Default/BlinnPhong/BlinnPhong.vert.spv"));
    Simple.loadFromFile(Paths::GetAssetFullPath("Models/retroufo.gltf"),vulkanDevice,GraphicsQueue,glTFLoadingFlags);
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
    
    layout = new vkb::PipelineLayout{*vulkanDevice,shaders};
    
    Pool1 = new vkb::DescriptorPool{*vulkanDevice,layout->get_descriptor_set_layout(0)};
    Pool2 = new vkb::DescriptorPool{*vulkanDevice,layout->get_descriptor_set_layout(1)};
    Pool3 = new vkb::DescriptorPool{*vulkanDevice,layout->get_descriptor_set_layout(2)};
    
    uboScene = new vkb::Buffer(*vulkanDevice,sizeof(UboScene)
    ,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,VMA_MEMORY_USAGE_CPU_TO_GPU ,VMA_ALLOCATION_CREATE_MAPPED_BIT |
                                            VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);

    uboLight = new vkb::Buffer(*vulkanDevice,sizeof(UboLight)
    ,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,VMA_MEMORY_USAGE_CPU_TO_GPU ,VMA_ALLOCATION_CREATE_MAPPED_BIT |
                                            VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);

    uboMaterial = new vkb::Buffer(*vulkanDevice,sizeof(UboMaterial)
    ,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,VMA_MEMORY_USAGE_CPU_TO_GPU,VMA_ALLOCATION_CREATE_MAPPED_BIT |
                                            VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT );

    
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = uboScene->GetHandle();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UboScene);   
    BindingMap<VkDescriptorBufferInfo> SceneInfo{};
    SceneInfo[0][0] = bufferInfo;
    DescriptorSet1 = new vkb::DescriptorSet{*vulkanDevice,layout->get_descriptor_set_layout(0),*Pool1,SceneInfo};
    DescriptorSet1->update({0});
    
    bufferInfo.buffer = uboLight->GetHandle();
    bufferInfo.range = sizeof(UboLight);
    BindingMap<VkDescriptorBufferInfo> LightInfo{};
    LightInfo[0][0] = bufferInfo;
    DescriptorSet2 = new vkb::DescriptorSet{*vulkanDevice,layout->get_descriptor_set_layout(1),*Pool2,LightInfo};
    DescriptorSet2->update({0});

    bufferInfo.buffer = uboMaterial->GetHandle();
    bufferInfo.range = sizeof(UboMaterial);
    BindingMap<VkDescriptorBufferInfo> MaterialInfo{};
    MaterialInfo[0][0] = bufferInfo;
    DescriptorSet3 = new vkb::DescriptorSet{*vulkanDevice,layout->get_descriptor_set_layout(2),*Pool3,MaterialInfo};
    DescriptorSet3->update({0});

    
    vkb::PipelineState state;
    vkb::VertexInputState vertexInputState;
    vertexInputState.bindings.push_back(vkglTF::Vertex::inputBindingDescription(0));
    vertexInputState.attributes = vkglTF::Vertex::inputAttributeDescriptions(0,{vkglTF::VertexComponent::Position, vkglTF::VertexComponent::Normal, vkglTF::VertexComponent::UV});
    state.set_vertex_input_state(vertexInputState);

    vkb::ColorBlendState color_blend_state;
    vkb::ColorBlendAttachmentState blend_attachment_state;
    color_blend_state.attachments.push_back(blend_attachment_state);
    state.set_color_blend_state(color_blend_state);
    state.set_pipeline_layout(*layout);

    std::vector<vkb::Attachment> attachments = {
        // 颜色附件
        {
            VK_FORMAT_R8G8B8A8_UNORM,       // format
            VK_SAMPLE_COUNT_1_BIT,          // samples (单采样)
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT// usage
        },
        // 深度附件 (可选)
        {
            VK_FORMAT_D32_SFLOAT,           // format
            VK_SAMPLE_COUNT_1_BIT,          // samples (单采样)
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, // usage
        }
    };
    std::vector<vkb::LoadStoreInfo> load_store_infos = {
        // 颜色附件
        {
            VK_ATTACHMENT_LOAD_OP_CLEAR,    // 开始渲染前清除附件
            VK_ATTACHMENT_STORE_OP_STORE    // 渲染后保存结果
        },
        // 深度附件
        {
            VK_ATTACHMENT_LOAD_OP_CLEAR,    // 开始渲染前清除深度
            VK_ATTACHMENT_STORE_OP_DONT_CARE // 渲染后不关心深度数据(如果不需要后续使用)
        }
    };

    std::vector<vkb::SubpassInfo> subpasses = {
        {
            {},                             // 无输入附件
            {0},                            // 输出到第一个颜色附件
            {},                             // 无颜色解析附件(不使用多重采样)
            false,                          // 不禁用深度模板附件
            VK_ATTACHMENT_UNUSED,           // 无深度模板解析附件
            VK_RESOLVE_MODE_NONE,           // 无解析模式
            "Main subpass"                  // 调试名称
        }
    };
    
    render_pass = new vkb::RenderPass{*vulkanDevice, attachments, load_store_infos, subpasses};
    
    state.set_render_pass(*render_pass);
    
    pipeline = new vkb::GraphicsPipeline{*vulkanDevice,VK_NULL_HANDLE, state};

    
    /*vkb::Image OffScreenImage{*vulkanDevice,vkb::ImageBuilder{ 512,512,1}
        .with_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT)
    .with_format(VK_FORMAT_R8G8B8A8_UNORM)};*/

    std::vector<vkb::Image> images;
    images.emplace_back(*vulkanDevice,vkb::ImageBuilder{ 512,512,1}
        .with_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
        .with_format(VK_FORMAT_R8G8B8A8_UNORM));
    images.emplace_back(*vulkanDevice,vkb::ImageBuilder{ 512,512,1}
        .with_usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        .with_format(VK_FORMAT_D32_SFLOAT));
    
    
    OffScreenRT = new vkb::RenderTarget{std::move(images)};
     
    OffScreenFB = new vkb::Framebuffer{*vulkanDevice,*OffScreenRT,*render_pass};
    OffScreenResourcesReady = true;
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
    GlobalUI->draw(true);

    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
    VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[currentBuffer], &cmdBufInfo));
    bool OffScreenImageReady = false;
    if (OffScreenSize.x > 256 && OffScreenSize.y > 256 && OffScreenResourcesReady){
        VkClearValue clearValues[2];
        clearValues[0].color = { { 0.2f, 0.2f, 0.2f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
        renderPassBeginInfo.renderPass = render_pass->GetHandle();
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = static_cast<uint32_t>(OffScreenSize.x);
        renderPassBeginInfo.renderArea.extent.height = static_cast<uint32_t>(OffScreenSize.y);
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = clearValues;
    
        renderPassBeginInfo.framebuffer = OffScreenFB->get_handle();// TODO
        //vkb::image_layout_transition(drawCmdBuffers[currentBuffer], OffScreenRT->get_views()[0].get_image().GetHandle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);


        vkCmdBeginRenderPass(drawCmdBuffers[currentBuffer], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = vks::initializers::viewport(OffScreenSize.x, OffScreenSize.y, 0.0f, 1.0f);
        vkCmdSetViewport(drawCmdBuffers[currentBuffer], 0, 1, &viewport);

        VkRect2D scissor = vks::initializers::rect2D(OffScreenSize.x, OffScreenSize.y, 0, 0);
        vkCmdSetScissor(drawCmdBuffers[currentBuffer], 0, 1, &scissor);

        // Render scene
        std::vector<VkDescriptorSet> descriptorSets = {DescriptorSet1->get_handle(), DescriptorSet2->get_handle(), DescriptorSet3->get_handle() };
        vkCmdBindDescriptorSets(drawCmdBuffers[currentBuffer], VK_PIPELINE_BIND_POINT_GRAPHICS, layout->get_handle(), 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
        vkCmdBindPipeline(drawCmdBuffers[currentBuffer], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_handle());

        vkCmdPushConstants(
        drawCmdBuffers[currentBuffer],              // 命令缓冲区
        layout->get_handle(),             // 管线布局
        VK_SHADER_STAGE_VERTEX_BIT, // 着色器阶段（根据实际使用调整）
        0,                         // 偏移量
        sizeof(PushConstants),     // 数据大小
        &Constant                  // 数据指针（使用你的结构体实例）
        );
        
        Simple.draw(drawCmdBuffers[currentBuffer]);
    
        vkCmdEndRenderPass(drawCmdBuffers[currentBuffer]);
        //vkb::image_layout_transition(drawCmdBuffers[currentBuffer], OffScreenRT->get_views()[0].get_image().GetHandle(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        OffScreenImageReady = true;
    }

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
    
 
    // Set target frame buffer
    renderPassBeginInfo.framebuffer = frameBuffers[currentBuffer];

    vkCmdBeginRenderPass(drawCmdBuffers[currentBuffer], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), drawCmdBuffers[currentBuffer]);

    vkCmdEndRenderPass(drawCmdBuffers[currentBuffer]);

    VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[currentBuffer]));
    
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
    
    UpdateUnifrom();



    
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