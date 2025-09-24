#include "Render/RenderSystem.hpp"

#include "Framework/Core/CommandBuffer.hpp"
#include "Framework/Platform/Window.hpp"
#include "Framework/Rendering/RenderFrame.hpp"

bool RenderSystem::Prepare(const ApplicationOptions& options)
{
    LOG_INFO("Initializing vulkan render system!")
    assert(options.window != nullptr && "Window is invalid");
    window = options.window;

    bool headless = window->get_window_mode() == vkb::Window::Mode::Headless;

    VK_CHECK_RESULT(volkInitialize());

    // Creating the vulkan instance
    for (const char* extension_name : window->get_required_surface_extensions())
    {
        AddInstanceExtension(extension_name);
    }

#ifdef DEBUG
    {
        std::vector<vk::ExtensionProperties> available_instance_extensions = vk::enumerateInstanceExtensionProperties();
        auto debugExtensionIt =
            std::find_if(available_instance_extensions.begin(), available_instance_extensions.end(),
                         [](vk::ExtensionProperties const& ep)
                         {
                             return strcmp(ep.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0;
                         });
        if (debugExtensionIt != available_instance_extensions.end())
        {
            LOGI("Vulkan debug utils enabled ({})", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

            debug_utils = std::make_unique<vkb::DebugUtilsExtDebugUtils>();
            AddInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
    }
#endif

    instance = CreateInstance();

    surface = window->create_surface(*instance);
    if (!surface)
    {
        throw std::runtime_error("Failed to create window surface.");
    }

    auto& gpu = instance->get_suitable_gpu(surface, headless);
    gpu.set_high_priority_graphics_queue_enable(high_priority_graphics_queue);

    if (gpu.get_features().textureCompressionASTC_LDR)
    {
        gpu.get_mutable_requested_features().textureCompressionASTC_LDR = true;
    }

    RequestGpuFeatures(gpu);

    // Creating vulkan device, specifying the swapchain extension always
    // If using VK_EXT_headless_surface, we still create and use a swap-chain
    {
        AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        if (instance_extensions.find(VK_KHR_DISPLAY_EXTENSION_NAME) != instance_extensions.end())
        {
            AddDeviceExtension(VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME, /*optional=*/true);
        }
    }
    // TODO
#ifdef VKB_ENABLE_PORTABILITY
    // VK_KHR_portability_subset must be enabled if present in the implementation (e.g on macOS/iOS with beta extensions enabled)
    add_device_extension(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME, /*optional=*/true);
#endif

#ifdef DEBUG
    if (!debug_utils)
    {
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(gpu.get_handle(), nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> available_device_extensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(gpu.get_handle(), nullptr, &extensionCount,
                                             available_device_extensions.data());
        auto debugExtensionIt =
            std::find_if(available_device_extensions.begin(),
                         available_device_extensions.end(),
                         [](const VkExtensionProperties& ep)
                         {
                             return strcmp(ep.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0;
                         });
        if (debugExtensionIt != available_device_extensions.end())
        {
            LOGI("Vulkan debug utils enabled ({})", VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
            AddDeviceExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        }
    }

    if (!debug_utils)
    {
        LOGW("Vulkan debug utils were requested, but no extension that provides them was found");
    }
#endif

    if (!debug_utils)
    {
        debug_utils = std::make_unique<vkb::DummyDebugUtils>();
    }
    device = CreateDevice(gpu);

    CreateRenderContext();
    render_context->prepare();

    // stats = std::make_unique<vkb::stats::HPPStats>(*render_context);

    // Start the sample in the first GUI configuration
    // configuration.reset();
}

void RenderSystem::RequestGpuFeatures(vkb::PhysicalDevice& gpu)
{
    // To be overridden by sample
}

std::unique_ptr<vkb::Instance> RenderSystem::CreateInstance()
{
    return std::make_unique<vkb::Instance>("VulkanRenderer", GetInstanceExtensions(), GetInstanceLayers(),
                                           GetLayerSettings(), api_version);
}

std::unique_ptr<vkb::VulkanDevice> RenderSystem::CreateDevice(vkb::PhysicalDevice& gpu)
{
    return std::make_unique<vkb::VulkanDevice>(gpu, surface, std::move(debug_utils), GetDeviceExtensions());
}

void RenderSystem::Draw(vkb::CommandBuffer& command_buffer, vkb::RenderTarget& render_target)
{
    auto& views = render_target.get_views();

    {
        // Image 0 is the swapchain
        vkb::ImageMemoryBarrier memory_barrier{};
        memory_barrier.old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        memory_barrier.new_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        memory_barrier.src_access_mask = 0;
        memory_barrier.dst_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        memory_barrier.src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        memory_barrier.dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        command_buffer.image_memory_barrier(views[0], memory_barrier);
        render_target.set_layout(0, memory_barrier.new_layout);

        // Skip 1 as it is handled later as a depth-stencil attachment
        for (size_t i = 2; i < views.size(); ++i)
        {
            command_buffer.image_memory_barrier(views[i], memory_barrier);
            render_target.set_layout(static_cast<uint32_t>(i), memory_barrier.new_layout);
        }
    }

    {
        vkb::ImageMemoryBarrier memory_barrier{};
        memory_barrier.old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        memory_barrier.new_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        memory_barrier.src_access_mask = 0;
        memory_barrier.dst_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        memory_barrier.src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        memory_barrier.dst_stage_mask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

        command_buffer.image_memory_barrier(views[1], memory_barrier);
        render_target.set_layout(1, memory_barrier.new_layout);
    }

    // draw_renderpass is a virtual function, thus we have to call that, instead of directly calling draw_renderpass_impl!
    DrawRenderpass(command_buffer, render_target);


    {
        vkb::ImageMemoryBarrier memory_barrier{};
        memory_barrier.old_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        memory_barrier.new_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        memory_barrier.src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        memory_barrier.src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        memory_barrier.dst_stage_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

        command_buffer.image_memory_barrier(views[0], memory_barrier);
        render_target.set_layout(0, memory_barrier.new_layout);
    }
}

void RenderSystem::Render(vkb::CommandBuffer& command_buffer)
{
    if (render_pipeline)
    {
        render_pipeline->draw(command_buffer, render_context->get_active_frame().get_render_target());
    }
}

void RenderSystem::DrawRenderpass(vkb::CommandBuffer& command_buffer, vkb::RenderTarget& render_target)
{
    SetViewportAndScissor(command_buffer, render_target.get_extent());

    // render is a virtual function, thus we have to call that, instead of directly calling render_impl!
    Render(command_buffer);


    vkCmdEndRenderPass(command_buffer.GetHandle());
}

void RenderSystem::Update(float delta_time)
{
    UpdateScene(delta_time);

    //update_gui(delta_time);

    auto command_buffer = render_context->begin();

    // Collect the performance data for the sample graphs
    //update_stats(delta_time);

    command_buffer->begin(VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    //stats->begin_sampling(*command_buffer);

    Draw(*command_buffer, render_context->get_active_frame().get_render_target());

    //stats->end_sampling(*command_buffer);
    command_buffer->end();

    render_context->submit(command_buffer);
}

void RenderSystem::UpdateScene(float delta_time)
{
    /*if (scene)
    {
        // Update scripts
        if (scene->has_component<sg::Script>())
        {
            auto scripts = scene->get_components<sg::Script>();

            for (auto script : scripts)
            {
                script->update(delta_time);
            }
        }

        // Update animations
        if (scene->has_component<sg::Animation>())
        {
            auto animations = scene->get_components<sg::Animation>();

            for (auto animation : animations)
            {
                animation->update(delta_time);
            }
        }
    }*/
}

void RenderSystem::UpdateDebugWindow()
{
    /*auto        driver_version     = device->get_gpu().get_driver_version();
    std::string driver_version_str = fmt::format("major: {} minor: {} patch: {}", driver_version.major, driver_version.minor, driver_version.patch);

    get_debug_info().template insert<field::Static, std::string>("driver_version", driver_version_str);
    get_debug_info().template insert<field::Static, std::string>("resolution",
                                                                 to_string(static_cast<VkExtent2D const &>(render_context->get_swapchain().get_extent())));
    get_debug_info().template insert<field::Static, std::string>("surface_format",
                                                                 to_string(render_context->get_swapchain().get_format()) + " (" +
                                                                     to_string(vkb::common::get_bits_per_pixel(render_context->get_swapchain().get_format())) +
                                                                     "bpp)");

    if (scene != nullptr)
    {
        get_debug_info().template insert<field::Static, uint32_t>("mesh_count", to_u32(scene->get_components<sg::SubMesh>().size()));
        get_debug_info().template insert<field::Static, uint32_t>("texture_count", to_u32(scene->get_components<sg::Texture>().size()));

        if (auto camera = scene->get_components<vkb::sg::Camera>()[0])
        {
            if (auto camera_node = camera->get_node())
            {
                const glm::vec3 &pos = camera_node->get_transform().get_translation();
                get_debug_info().template insert<field::Vector, float>("camera_pos", pos.x, pos.y, pos.z);
            }
        }
    }*/
}

void RenderSystem::Finish()
{
    if (device)
    {
        vkDeviceWaitIdle(device->GetHandle());
    }
}


void RenderSystem::SetViewportAndScissor(vkb::CommandBuffer const& command_buffer, VkExtent2D const& extent)
{
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer.GetHandle(), 0, 1, &viewport);

    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(command_buffer.GetHandle(), 0, 1, &scissor);
}

void RenderSystem::CreateRenderContext()
{
    CreateRenderContext_Impl(surface_priority_list);
}

void RenderSystem::CreateRenderContext_Impl(const std::vector<VkSurfaceFormatKHR>& surface_priority_list)
{
    VkPresentModeKHR present_mode = (window->get_properties().vsync == vkb::Window::Vsync::ON)
                                        ? VK_PRESENT_MODE_FIFO_KHR
                                        : VK_PRESENT_MODE_MAILBOX_KHR;
    std::vector<VkPresentModeKHR> present_mode_priority_list{
        VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR
    };

    render_context =
        std::make_unique<vkb::RenderContext>(*device, surface, *window, present_mode,
                                             present_mode_priority_list, surface_priority_list);
}

void RenderSystem::ResetStatsView()
{
}

bool RenderSystem::Resize(uint32_t width, uint32_t height)
{
    return false;
    /*if (!Parent::resize(width, height))
    {
        return false;
    }

    if (gui)
    {
        gui->resize(width, height);
    }

    if (scene && scene->has_component<sg::Script>())
    {
        auto scripts = scene->get_components<sg::Script>();

        for (auto script : scripts)
        {
            script->resize(width, height);
        }
    }

    if (stats)
    {
        stats->resize(width);
    }
    return true;*/
}

void RenderSystem::SetApiVersion(uint32_t requested_api_version)
{
    api_version = requested_api_version;
}

void RenderSystem::SetRenderContext(std::unique_ptr<vkb::RenderContext>&& rc)
{
    render_context.reset(rc.release());
}

void RenderSystem::SetRenderPipeline(std::unique_ptr<vkb::RenderPipeline>&& rp)
{
    render_pipeline.reset(rp.release());
}

void RenderSystem::AddDeviceExtension(const char* extension, bool optional)
{
    device_extensions[extension] = optional;
}

void RenderSystem::AddInstanceExtension(const char* extension, bool optional)
{
    instance_extensions[extension] = optional;
}

std::unordered_map<const char*, bool> const& RenderSystem::GetDeviceExtensions() const
{
    return device_extensions;
}

std::unordered_map<const char*, bool> const& RenderSystem::GetInstanceExtensions() const
{
    return instance_extensions;
}

std::unordered_map<const char*, bool> const& RenderSystem::GetInstanceLayers() const
{
    return instance_layers;
}

std::vector<VkLayerSettingEXT> const& RenderSystem::GetLayerSettings() const
{
    return layer_settings;
}

std::vector<VkSurfaceFormatKHR> const& RenderSystem::GetSurfacePriorityList() const
{
    return surface_priority_list;
}

std::vector<VkSurfaceFormatKHR>& RenderSystem::GetSurfacePriorityList()
{
    return surface_priority_list;
}
