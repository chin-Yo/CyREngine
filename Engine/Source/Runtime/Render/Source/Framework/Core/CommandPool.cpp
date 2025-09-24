#include "Framework/Core/CommandPool.hpp"
#include "Framework/Core/CommandBuffer.hpp"
#include "Framework/Core/VulkanDevice.hpp"

namespace vkb
{
    CommandPool::CommandPool(vkb::VulkanDevice& device_,
                             uint32_t queue_family_index_,
                             vkb::RenderFrame* render_frame_,
                             size_t thread_index_,
                             vkb::CommandBufferResetMode reset_mode_) : device{device_},
                                                                        render_frame{render_frame_},
                                                                        thread_index{thread_index_},
                                                                        queue_family_index{queue_family_index_},
                                                                        reset_mode{reset_mode_}
    {
        VkCommandPoolCreateFlags flags = 0;
        switch (reset_mode)
        {
        case vkb::CommandBufferResetMode::ResetIndividually:
        case vkb::CommandBufferResetMode::AlwaysAllocate:
            flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            break;
        case vkb::CommandBufferResetMode::ResetPool:
        default:
            flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            break;
        }
        VkCommandPoolCreateInfo command_pool_create_info{};
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.pNext = nullptr;
        command_pool_create_info.flags = flags;
        command_pool_create_info.queueFamilyIndex = queue_family_index;

        VkResult result = vkCreateCommandPool(device.GetHandle(), &command_pool_create_info, nullptr, &handle);

        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create command pool");
        }
    }

    CommandPool::CommandPool(CommandPool&& other) noexcept : device{other.device},
                                                             handle{other.handle},
                                                             render_frame{other.render_frame},
                                                             thread_index{other.thread_index},
                                                             queue_family_index{other.queue_family_index},
                                                             primary_command_buffers{
                                                                 std::move(other.primary_command_buffers)
                                                             },
                                                             active_primary_command_buffer_count{
                                                                 other.active_primary_command_buffer_count
                                                             },
                                                             secondary_command_buffers{
                                                                 std::move(other.secondary_command_buffers)
                                                             },
                                                             active_secondary_command_buffer_count{
                                                                 other.active_secondary_command_buffer_count
                                                             },
                                                             reset_mode{other.reset_mode}
    {
        other.handle = VK_NULL_HANDLE;
    }

    CommandPool::~CommandPool()
    {
        primary_command_buffers.clear();
        secondary_command_buffers.clear();
        if (handle != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(device.GetHandle(), handle, nullptr);
        }
    }

    vkb::VulkanDevice& CommandPool::get_device()
    {
        return device;
    }

    VkCommandPool CommandPool::get_handle() const
    {
        return handle;
    }

    uint32_t CommandPool::get_queue_family_index() const
    {
        return queue_family_index;
    }

    vkb::RenderFrame* CommandPool::get_render_frame()
    {
        return render_frame;
    }

    vkb::CommandBufferResetMode CommandPool::get_reset_mode() const
    {
        return reset_mode;
    }

    size_t CommandPool::get_thread_index() const
    {
        return thread_index;
    }

    std::shared_ptr<vkb::CommandBuffer> CommandPool::request_command_buffer(VkCommandBufferLevel level)
    {
        return CommandPool::request_command_buffer(*this, level);
    }

    std::shared_ptr<vkb::CommandBuffer> CommandPool::request_command_buffer(
        vkb::CommandPool& commandPool, VkCommandBufferLevel level)
    {
        if (level == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
        {
            if (active_primary_command_buffer_count < primary_command_buffers.size())
            {
                return primary_command_buffers[active_primary_command_buffer_count++];
            }

            primary_command_buffers.emplace_back(std::make_shared<vkb::CommandBuffer>(commandPool, level));
            active_primary_command_buffer_count++;
            return primary_command_buffers.back();
        }
        else
        {
            if (active_secondary_command_buffer_count < secondary_command_buffers.size())
            {
                return secondary_command_buffers[active_secondary_command_buffer_count++];
            }

            secondary_command_buffers.emplace_back(std::make_shared<vkb::CommandBuffer>(commandPool, level));
            active_secondary_command_buffer_count++;
            return secondary_command_buffers.back();
        }
    }

    void CommandPool::reset_pool()
    {
        switch (reset_mode)
        {
        case vkb::CommandBufferResetMode::ResetIndividually:
            for (auto& cmd_buf : primary_command_buffers)
            {
                cmd_buf->reset(reset_mode);
            }
            active_primary_command_buffer_count = 0;

            for (auto& cmd_buf : secondary_command_buffers)
            {
                cmd_buf->reset(reset_mode);
            }
            active_secondary_command_buffer_count = 0;
            break;
        case vkb::CommandBufferResetMode::ResetPool:
            {
                VkResult result = vkResetCommandPool(device.GetHandle(), handle, 0);
                if (result != VK_SUCCESS)
                {
                    // TODO LOG
                }
                active_primary_command_buffer_count = 0;
                active_secondary_command_buffer_count = 0;
                break;
            }
        case vkb::CommandBufferResetMode::AlwaysAllocate:
            primary_command_buffers.clear();
            active_primary_command_buffer_count = 0;
            secondary_command_buffers.clear();
            active_secondary_command_buffer_count = 0;
            break;
        default:
            throw std::runtime_error("Unknown reset mode for command pools");
        }
    }
} // namespace vkb
