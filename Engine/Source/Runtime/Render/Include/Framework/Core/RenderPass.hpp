#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

namespace vks
{
    class RenderPass
    {
    public:
        // 构造函数，接收一个已经创建好的 VkRenderPass
        RenderPass(VkDevice device, VkRenderPass renderPass);
        ~RenderPass();

        // 禁止拷贝，但允许移动
        RenderPass(const RenderPass &) = delete;
        RenderPass &operator=(const RenderPass &) = delete;
        RenderPass(RenderPass &&other) noexcept;
        RenderPass &operator=(RenderPass &&other) noexcept;

        // 获取底层的 Vulkan 句柄
        VkRenderPass get() const { return m_renderPass; }
        operator VkRenderPass() const { return m_renderPass; } // 隐式转换

    private:
        VkDevice m_device = VK_NULL_HANDLE;
        VkRenderPass m_renderPass = VK_NULL_HANDLE;
    };

    class RenderPassBuilder
    {
    public:
        RenderPassBuilder(VkDevice device);

        // 添加附件 (Attachment)
        RenderPassBuilder &addAttachment(
            VkFormat format,
            VkSampleCountFlagBits samples,
            VkAttachmentLoadOp loadOp,
            VkAttachmentStoreOp storeOp,
            VkImageLayout initialLayout,
            VkImageLayout finalLayout);

        // 添加子流程 (Subpass)
        // 使用附件的索引来指定 color/depth attachments
        RenderPassBuilder &addSubpass(
            VkPipelineBindPoint bindPoint,
            const std::vector<uint32_t> &colorAttachmentIndices,
            std::optional<uint32_t> depthAttachmentIndex = std::nullopt);

        // 添加子流程依赖 (Dependency)
        RenderPassBuilder &addDependency(
            uint32_t srcSubpass,
            uint32_t dstSubpass,
            VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags dstStageMask,
            VkAccessFlags srcAccessMask,
            VkAccessFlags dstAccessMask,
            VkDependencyFlags dependencyFlags = 0);

        // 构建 RenderPass 对象
        RenderPass build();

    private:
        VkDevice m_device;

        // 这些 vector 用于存储所有配置信息。
        // Builder 对象持有它们的所有权，确保在调用 vkCreateRenderPass 时指针有效。
        std::vector<VkAttachmentDescription> m_attachments;
        std::vector<VkSubpassDescription> m_subpasses;
        std::vector<VkSubpassDependency> m_dependencies;

        // 这是关键：VkSubpassDescription 内部的指针需要指向有效内存。
        // 我们用 vector 来存储这些引用，确保它们在 Builder 的生命周期内都有效。
        std::vector<std::vector<VkAttachmentReference>> m_subpassColorAttachmentRefs;
        std::vector<std::optional<VkAttachmentReference>> m_subpassDepthAttachmentRefs;
    };
}