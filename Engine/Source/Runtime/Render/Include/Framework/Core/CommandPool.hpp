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

namespace vks
{
    /**
     * @class CommandPool
     * @brief 一个 RAII 包装器，用于管理 VkCommandPool 的生命周期。
     * 
     * 这个类持有 VkCommandPool 句柄，并在其析构时自动销毁它。
     * 它被设计为不可拷贝但可移动，以确保单一所有权。
     * 它还提供了与命令池交互的核心功能，如分配和释放命令缓冲区。
     * 这个类的实例应该由 CommandPoolBuilder 创建。
     */
    class CommandPool
    {
    public:
        // 构造函数由 CommandPoolBuilder 调用
        CommandPool(VkDevice device, VkCommandPool commandPool);
        ~CommandPool();

        // 禁止拷贝，保证资源所有权唯一
        CommandPool(const CommandPool&) = delete;
        CommandPool& operator=(const CommandPool&) = delete;

        // 允许移动，用于所有权转移
        CommandPool(CommandPool&& other) noexcept;
        CommandPool& operator=(CommandPool&& other) noexcept;

        /**
         * @brief 从池中分配一个或多个命令缓冲区。
         * @param count 要分配的命令缓冲区数量。
         * @param level 命令缓冲区的级别 (PRIMARY 或 SECONDARY)。
         * @return 包含已分配的 VkCommandBuffer 句柄的 vector。如果分配失败则抛出异常。
         */
        [[nodiscard]] std::vector<VkCommandBuffer> allocateCommandBuffers(
            uint32_t count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        /**
         * @brief 释放之前从该池中分配的命令缓冲区。
         * @param commandBuffers 要释放的命令缓冲区列表。
         */
        void freeCommandBuffers(const std::vector<VkCommandBuffer>& commandBuffers);

        /**
         * @brief 重置整个命令池，将其所有命令缓冲区回收以重新使用。
         * @param flags VkCommandPoolResetFlags 的组合。
         */
        void reset(VkCommandPoolResetFlags flags = 0);

        //==================== 访问器 ====================//

        VkCommandPool get() const { return m_commandPool; }
        operator VkCommandPool() const { return m_commandPool; }

    private:
        void destroy();

        VkDevice m_device = VK_NULL_HANDLE;
        VkCommandPool m_commandPool = VK_NULL_HANDLE;
    };

    /**
     * @class CommandPoolBuilder
     * @brief 使用链式调用风格来配置和创建 CommandPool 实例。
     * 
     * 这个类收集创建 VkCommandPool所需的所有信息，
     * 并通过 build() 方法最终创建一个 CommandPool 对象。
     */
    class CommandPoolBuilder
    {
    public:
        explicit CommandPoolBuilder(VkDevice device);

        //==================== 链式配置方法 ====================//

        /**
         * @brief 设置命令池将要服务的目标队列族索引。这是必需的设置。
         */
        CommandPoolBuilder& setQueueFamilyIndex(uint32_t queueFamilyIndex);

        /**
         * @brief 设置命令池的创建标志。
         */
        CommandPoolBuilder& setFlags(VkCommandPoolCreateFlags flags);

        /**
         * @brief 便捷方法：设置 VK_COMMAND_POOL_CREATE_TRANSIENT_BIT 标志。
         *        提示命令缓冲区是短暂的，用于优化内存分配。
         */
        CommandPoolBuilder& setTransient();

        /**
         * @brief 便捷方法：设置 VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT 标志。
         *        允许单个命令缓冲区被重置，而不是整个池。
         */
        CommandPoolBuilder& setResetCommandBuffer();

        //==================== 构建方法 ====================//

        /**
         * @brief 根据配置创建并返回一个 CommandPool 对象。
         * @return 一个拥有新创建的 VkCommandPool 的 CommandPool 实例。
         * @throws std::runtime_error 如果 vkCreateCommandPool 失败。
         */
        [[nodiscard]] CommandPool build() const;

        /**
         * @brief 构建并返回 VkCommandPoolCreateInfo 结构体。
         *        这对于需要 create info 但不想立即创建对象的场景很有用。
         */
        [[nodiscard]] VkCommandPoolCreateInfo buildCreateInfo() const;

    private:
        VkDevice m_device;
        VkCommandPoolCreateInfo m_createInfo{};
    };
}
