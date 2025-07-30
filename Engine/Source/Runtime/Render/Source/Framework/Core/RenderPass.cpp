#include "Framework/Core/RenderPass.hpp"
#include <stdexcept>

namespace vks
{
    RenderPass::RenderPass(VkDevice device, VkRenderPass renderPass)
        : m_device(device), m_renderPass(renderPass) {}

    RenderPass::~RenderPass()
    {
        if (m_renderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(m_device, m_renderPass, nullptr);
        }
    }

    RenderPass::RenderPass(RenderPass &&other) noexcept
        : m_device(other.m_device), m_renderPass(other.m_renderPass)
    {
        other.m_renderPass = VK_NULL_HANDLE; // 避免双重释放
    }

    RenderPassBuilder::RenderPassBuilder(VkDevice device) : m_device(device) {}

    RenderPassBuilder &RenderPassBuilder::addAttachment(
        VkFormat format, VkSampleCountFlagBits samples,
        VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
        VkImageLayout initialLayout, VkImageLayout finalLayout)
    {

        VkAttachmentDescription attachment{};
        attachment.format = format;
        attachment.samples = samples;
        attachment.loadOp = loadOp;
        attachment.storeOp = storeOp;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = initialLayout;
        attachment.finalLayout = finalLayout;

        m_attachments.push_back(attachment);
        return *this; // 返回自身引用，实现链式调用
    }

    RenderPassBuilder &RenderPassBuilder::addSubpass(
        VkPipelineBindPoint bindPoint,
        const std::vector<uint32_t> &colorAttachmentIndices,
        std::optional<uint32_t> depthAttachmentIndex)
    {

        // 为当前 subpass 创建并存储颜色附件引用
        std::vector<VkAttachmentReference> colorRefs;
        for (uint32_t index : colorAttachmentIndices)
        {
            colorRefs.push_back({index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        }
        m_subpassColorAttachmentRefs.push_back(std::move(colorRefs));

        // 为当前 subpass 创建并存储深度附件引用
        std::optional<VkAttachmentReference> depthRef = std::nullopt;
        if (depthAttachmentIndex.has_value())
        {
            depthRef = {depthAttachmentIndex.value(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
        }
        m_subpassDepthAttachmentRefs.push_back(depthRef);

        // 创建 subpass description
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = bindPoint;
        subpass.colorAttachmentCount = static_cast<uint32_t>(m_subpassColorAttachmentRefs.back().size());
        subpass.pColorAttachments = m_subpassColorAttachmentRefs.back().data();

        if (m_subpassDepthAttachmentRefs.back().has_value())
        {
            subpass.pDepthStencilAttachment = &m_subpassDepthAttachmentRefs.back().value();
        }
        else
        {
            subpass.pDepthStencilAttachment = nullptr;
        }

        m_subpasses.push_back(subpass);
        return *this;
    }

    RenderPassBuilder &RenderPassBuilder::addDependency(uint32_t srcSubpass, uint32_t dstSubpass,
                                                        VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                        VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkDependencyFlags dependencyFlags)
    {
        VkSubpassDependency dependency{};
        dependency.srcSubpass = srcSubpass;
        dependency.dstSubpass = dstSubpass;
        dependency.srcStageMask = srcStageMask;
        dependency.dstStageMask = dstStageMask;
        dependency.srcAccessMask = srcAccessMask;
        dependency.dstAccessMask = dstAccessMask;
        dependency.dependencyFlags = dependencyFlags;

        m_dependencies.push_back(dependency);
        return *this;
    }

    RenderPass RenderPassBuilder::build()
    {
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(m_attachments.size());
        renderPassInfo.pAttachments = m_attachments.data();
        renderPassInfo.subpassCount = static_cast<uint32_t>(m_subpasses.size());
        renderPassInfo.pSubpasses = m_subpasses.data();
        renderPassInfo.dependencyCount = static_cast<uint32_t>(m_dependencies.size());
        renderPassInfo.pDependencies = m_dependencies.data();

        VkRenderPass renderPass;
        if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create render pass!");
        }

        // 将创建好的 VkRenderPass 和 VkDevice 传递给 RAII 包装类
        return RenderPass(m_device, renderPass);
    }
}