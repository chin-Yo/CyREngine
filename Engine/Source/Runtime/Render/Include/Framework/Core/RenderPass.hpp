/* Copyright (c) 2019-2025, Arm Limited and Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 the "License";
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "Framework/Common/VkHelpers.hpp"
#include <volk.h>
#include <vector>
#include <optional>
#include "VulkanResource.hpp"
// #include "core/vulkan_resource.h"


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
        VkRenderPassCreateInfo buildCreateInfo();
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


namespace vkb
{
    struct Attachment;

    struct SubpassInfo
    {
        std::vector<uint32_t> input_attachments;

        std::vector<uint32_t> output_attachments;

        std::vector<uint32_t> color_resolve_attachments;

        bool disable_depth_stencil_attachment;

        uint32_t depth_stencil_resolve_attachment;

        VkResolveModeFlagBits depth_stencil_resolve_mode;

        std::string debug_name;
    };

    /**
     * @brief Load and store info for a render pass attachment.
     */
    struct LoadStoreInfo
    {
        VkAttachmentLoadOp load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;

        VkAttachmentStoreOp store_op = VK_ATTACHMENT_STORE_OP_STORE;
    };

    /*/**
     * @brief Description of render pass attachments.
     * Attachment descriptions can be used to automatically create render target images.
     #1#
    struct Attachment
    {
        VkFormat format{VK_FORMAT_UNDEFINED};

        VkSampleCountFlagBits samples{VK_SAMPLE_COUNT_1_BIT};

        VkImageUsageFlags usage{VK_IMAGE_USAGE_SAMPLED_BIT};

        VkImageLayout initial_layout{VK_IMAGE_LAYOUT_UNDEFINED};

        Attachment() = default;

        Attachment(VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage)
            : format(format),
              samples(samples),
              usage(usage)
        {
        }
    };*/

    class RenderPass : public VulkanResource<VkRenderPass>
    {
    public:
        RenderPass(VulkanDevice &device,
                   const std::vector<Attachment> &attachments,
                   const std::vector<LoadStoreInfo> &load_store_infos,
                   const std::vector<SubpassInfo> &subpasses);

        RenderPass(VulkanDevice &device, vks::RenderPassBuilder& builder);
        
        RenderPass(const RenderPass &) = delete;

        RenderPass(RenderPass &&other);

        ~RenderPass() override;

        RenderPass &operator=(const RenderPass &) = delete;

        RenderPass &operator=(RenderPass &&) = delete;

        const uint32_t get_color_output_count(uint32_t subpass_index) const;

        VkExtent2D get_render_area_granularity() const;

    private:
        size_t subpass_count;

        template <typename T_SubpassDescription, typename T_AttachmentDescription, typename T_AttachmentReference,
                  typename T_SubpassDependency, typename T_RenderPassCreateInfo>
        void create_renderpass(const std::vector<Attachment> &attachments,
                               const std::vector<LoadStoreInfo> &load_store_infos,
                               const std::vector<SubpassInfo> &subpasses);

        std::vector<uint32_t> color_output_count;
    };
} // namespace vkb
