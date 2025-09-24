#pragma once

#include <vector>
#include <memory>
#include <set>
#include "Framework/Rendering/RenderTarget.hpp"
#include "Framework/Core/SwapChain.hpp"

namespace vkb
{
    class CommandBuffer;
    class VulkanDevice;
    class Queue;
    class RenderFrame;
    class RenderTarget;
    class Swapchain;
    class Window;
    struct SwapchainProperties;

    enum class CommandBufferResetMode;
} // namespace vkb

namespace vkb
{
    /**
     * @class RenderContext
     * @brief 管理渲染帧，其生命周期与应用程序相同。
     *
     * RenderContext 作为一个帧管理器，负责管理 RenderFrame 对象，
     * 在它们之间切换并向活动帧转发Vulkan资源请求。
     * 它可以与交换链一起用于常规渲染，也可以在没有交换链的情况下用于离屏渲染。
     */
    class RenderContext
    {
    public:
        // 如果没有创建交换链，渲染目标将使用的默认格式
        static VkFormat DEFAULT_VK_FORMAT;

        /**
         * @brief 构造函数
         * @param device 一个有效的 VulkanDevice
         * @param surface 一个 surface，如果是离屏模式则为 VK_NULL_HANDLE
         * @param window 创建 surface 的窗口
         * @param present_mode 请求设置的交换链呈现模式
         * @param present_mode_priority_list 交换链选择呈现模式的优先级列表
         * @param surface_format_priority_list 交换链选择表面格式的优先级列表
         */
        RenderContext(VulkanDevice& device,
                      VkSurfaceKHR surface,
                      const Window& window,
                      VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR,
                      const std::vector<VkPresentModeKHR>& present_mode_priority_list = {
                          VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR
                      },
                      const std::vector<VkSurfaceFormatKHR>& surface_format_priority_list = {
                          {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                          {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}
                      });

        // 禁用拷贝构造和拷贝赋值
        RenderContext(const RenderContext&) = delete;
        RenderContext& operator=(const RenderContext&) = delete;

        // 禁用移动构造和移动赋值
        RenderContext(RenderContext&&) = delete;
        RenderContext& operator=(RenderContext&&) = delete;

        virtual ~RenderContext() = default;

        /**
         * @brief 准备渲染帧以进行渲染
         * @param thread_count 应用程序的线程数，用于为每个渲染帧分配资源池
         * @param create_render_target_func 用于创建 RenderTarget 的函数委托
         */
        void prepare(size_t thread_count = 1,
                     RenderTarget::CreateFunc create_render_target_func = RenderTarget::DEFAULT_CREATE_FUNC);

        /**
         * @brief 更新交换链的范围（如果存在）
         * @param extent 新交换链图像的宽度和高度
         */
        void update_swapchain(const VkExtent2D& extent);

        /**
         * @brief 更新交换链的图像数量（如果存在）
         * @param image_count 新交换链中的图像数量
         */
        void update_swapchain(const uint32_t image_count);

        /**
         * @brief 更新交换链的图像用法（如果存在）
         * @param image_usage_flags 新交换链图像将具有的用法标志
         */
        void update_swapchain(const std::set<VkImageUsageFlagBits>& image_usage_flags);

        /**
         * @brief 更新交换链的范围和表面变换（如果存在）
         * @param extent 新交换链图像的宽度和高度
         * @param transform 表面变换标志
         */
        void update_swapchain(const VkExtent2D& extent, const VkSurfaceTransformFlagBitsKHR transform);

        /**
         * @brief 更新交换链的压缩设置（如果存在）
         * @param compression 用于交换链图像的压缩方式
         * @param compression_fixed_rate 如果是固定速率压缩，则指定速率
         */
        void update_swapchain(const VkImageCompressionFlagsEXT compression,
                              const VkImageCompressionFixedRateFlagsEXT compression_fixed_rate);

        /**
         * @brief 检查 RenderContext 中是否存在有效的交换链
         * @returns 如果存在有效交换链则返回 true
         */
        bool has_swapchain();

        /**
         * @brief 在每次更新后重新创建渲染帧
         */
        void recreate();

        /**
         * @brief 重新创建交换链
         */
        void recreate_swapchain();

        /**
         * @brief 准备下一个可用的帧进行渲染，并返回一个有效的命令缓冲区
         * @param reset_mode 命令缓冲区的重置方式
         * @returns 一个可用于记录命令的有效命令缓冲区
         */
        std::shared_ptr<vkb::CommandBuffer> begin(
            vkb::CommandBufferResetMode reset_mode = vkb::CommandBufferResetMode::ResetPool);

        /**
         * @brief 将单个命令缓冲区提交到正确的队列
         * @param command_buffer 包含已记录命令的命令缓冲区
         */
        void submit(std::shared_ptr<vkb::CommandBuffer> command_buffer);

        /**
         * @brief 将多个命令缓冲区提交到正确的队列
         * @param command_buffers 包含已记录命令的命令缓冲区列表
         */
        void submit(const std::vector<std::shared_ptr<vkb::CommandBuffer>>& command_buffers);

        /**
         * @brief 开始一帧的渲染
         */
        void begin_frame();

        /**
         * @brief 向指定队列提交命令，并处理信号量同步
         */
        VkSemaphore submit(const Queue& queue,
                           const std::vector<std::shared_ptr<vkb::CommandBuffer>>& command_buffers,
                           VkSemaphore wait_semaphore,
                           VkPipelineStageFlags wait_pipeline_stage);

        /**
         * @brief 向指定队列提交与帧相关的命令缓冲区
         */
        void submit(const Queue& queue, const std::vector<std::shared_ptr<vkb::CommandBuffer>>& command_buffers);

        /**
         * @brief 等待一帧完成其渲染
         */
        virtual void wait_frame();

        /**
         * @brief 结束一帧的渲染，并处理呈现
         */
        void end_frame(VkSemaphore semaphore);

        /**
         * @brief 获取当前活动的渲染帧
         * @return 对当前活动帧的引用
         */
        vkb::RenderFrame& get_active_frame();

        /**
         * @brief 获取当前活动帧的索引
         * @return 当前活动帧的索引
         */
        uint32_t get_active_frame_index();

        /**
         * @brief 获取上一个已渲染的帧
         * @return 对上一个渲染帧的引用
         */
        vkb::RenderFrame& get_last_rendered_frame();

        VkSemaphore request_semaphore();
        VkSemaphore request_semaphore_with_ownership();
        void release_owned_semaphore(VkSemaphore semaphore);

        VulkanDevice& get_device();

        /**
         * @brief 返回 RenderContext 中渲染目标的创建格式
         */
        VkFormat get_format() const;

        Swapchain const& get_swapchain() const;

        VkExtent2D const& get_surface_extent() const;

        uint32_t get_active_frame_index() const;

        std::vector<std::unique_ptr<vkb::RenderFrame>>& get_render_frames();

        /**
         * @brief 处理表面变化，仅在 RenderContext 使用交换链时适用
         */
        virtual bool handle_surface_changes(bool force_update = false);

        /**
         * @brief 获取并消耗WSI（窗口系统集成）的图像获取信号量。仅在特殊情况下使用。
         * @return WSI图像获取信号量
         */
        VkSemaphore consume_acquired_semaphore();

    protected:
        VkExtent2D surface_extent;

    private:
        VulkanDevice& device;

        const Window& window;

        /// 如果存在交换链，则为支持呈现的队列，否则为图形队列
        const Queue& queue;

        std::unique_ptr<Swapchain> swapchain;

        SwapchainProperties swapchain_properties;

        std::vector<std::unique_ptr<vkb::RenderFrame>> frames;

        VkSemaphore acquired_semaphore;

        bool prepared{false};

        /// 当前活动帧的索引
        uint32_t active_frame_index{0};

        /// 标记一帧是否处于活动状态
        bool frame_active{false};

        RenderTarget::CreateFunc create_render_target_func = RenderTarget::DEFAULT_CREATE_FUNC;

        VkSurfaceTransformFlagBitsKHR pre_transform{VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR};

        size_t thread_count{1};
    };
} // namespace vkb
