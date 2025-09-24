#pragma once

#include <cstdint>

#include "Framework/Common/VkCommon.hpp"


namespace vkb
{
    class VulkanDevice;
    // 前向声明重构后的类
    class RenderFrame;
    class CommandBuffer;
    class CommandPool;

    class CommandPool
    {
    public:
        CommandPool(vkb::VulkanDevice& device,
                    uint32_t queue_family_index,
                    vkb::RenderFrame* render_frame = nullptr,
                    size_t thread_index = 0,
                    vkb::CommandBufferResetMode reset_mode = vkb::CommandBufferResetMode::ResetPool);
        CommandPool(CommandPool const&) = delete;
        CommandPool(CommandPool&& other) noexcept;
        CommandPool& operator=(CommandPool const&) = delete;
        CommandPool& operator=(CommandPool&& other) = delete;
        ~CommandPool();


        vkb::VulkanDevice& get_device();
        VkCommandPool get_handle() const;
        uint32_t get_queue_family_index() const;
        vkb::RenderFrame* get_render_frame();
        vkb::CommandBufferResetMode get_reset_mode() const;
        size_t get_thread_index() const;

        std::shared_ptr<vkb::CommandBuffer> request_command_buffer(
            VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        // VkCommandBufferLevel指定命令缓冲区的层级，分为主层级（VK_COMMAND_BUFFER_LEVEL_PRIMARY，可独立执行或嵌套次级）和次级层级（VK_COMMAND_BUFFER_LEVEL_SECONDARY，仅能被主命令缓冲区调用）
        std::shared_ptr<vkb::CommandBuffer> request_command_buffer(vkb::CommandPool& commandPool,
                                                                   VkCommandBufferLevel level);
        void reset_pool();

    private:
        vkb::VulkanDevice& device;
        VkCommandPool handle = nullptr;
        vkb::RenderFrame* render_frame = nullptr;
        size_t thread_index = 0;
        uint32_t queue_family_index = 0;
        std::vector<std::shared_ptr<vkb::CommandBuffer>> primary_command_buffers;
        uint32_t active_primary_command_buffer_count = 0;
        std::vector<std::shared_ptr<vkb::CommandBuffer>> secondary_command_buffers;
        uint32_t active_secondary_command_buffer_count = 0;
        vkb::CommandBufferResetMode reset_mode = vkb::CommandBufferResetMode::ResetPool;
    };
} // namespace vkb
