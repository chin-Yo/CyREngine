#pragma once

#include <volk.h>

#include "Image.hpp"
#include "PipelineState.hpp"
#include "VulkanResource.hpp"

namespace vkb
{
	class Sampler;
	class RenderTarget;
	class QueryPool;
	struct LoadStoreInfo;
	class Framebuffer;

	class CommandBuffer
		: public VulkanResource<VkCommandBuffer>
	{
		using ParentType = vkb::VulkanResource<VkCommandBuffer>;
	public:
		using BufferImageCopyType       = VkBufferImageCopy;
		using ClearAttachmentType       = VkClearAttachment;
		using ClearRectType             = VkClearRect;
		using ClearValueType            = VkClearValue;
		using CommandBufferLevelType    = VkCommandBufferLevel;
		using CommandBufferType         = VkCommandBuffer;
		using CommandBufferUsageFlagsType = VkCommandBufferUsageFlags;
		using DeviceSizeType            = VkDeviceSize;
		using ImageBlitType             = VkImageBlit;
		using ImageCopyType             = VkImageCopy;
		using ImageLayoutType           = VkImageLayout;
		using ImageResolveType          = VkImageResolve;
		using IndexTypeType             = VkIndexType;
		using PipelineStagFlagBitsType  = VkPipelineStageFlagBits;
		using QueryControlFlagsType     = VkQueryControlFlags;
		using Rect2DType                = VkRect2D;
		using ResultType                = VkResult;
		using SubpassContentsType       = VkSubpassContents;
		using ViewportType              = VkViewport;

		using BufferMemoryBarrierType   = vkb::BufferMemoryBarrier;
		using ColorBlendStateType       = vkb::ColorBlendState;
		using DepthStencilStateType     = vkb::DepthStencilState;
		using FramebufferType           = vkb::Framebuffer;
		using ImageMemoryBarrierType    = vkb::ImageMemoryBarrier;
		using ImageType                 = vkb::Image;
		using ImageViewType             = vkb::ImageView;
		using InputAssemblyStateType    = vkb::InputAssemblyState;
		using LoadStoreInfoType         = vkb::LoadStoreInfo;
		using MultisampleStateType      = vkb::MultisampleState;
		using PipelineLayoutType        = vkb::PipelineLayout;
		using QueryPoolType             = vkb::QueryPool;
		using RasterizationStateType    = vkb::RasterizationState;
		using RenderPassType            = vkb::RenderPass;
		using RenderTargetType          = vkb::RenderTarget;
		using SamplerType               = vkb::Sampler;
		using VertexInputStateType      = vkb::VertexInputState;
		using ViewportStateType         = vkb::ViewportState;

		// ... 类的其余成员函数和变量 ...
	};
}
