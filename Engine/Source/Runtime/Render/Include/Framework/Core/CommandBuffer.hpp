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
		using BufferImageCopyType = VkBufferImageCopy;
		using ClearAttachmentType = VkClearAttachment;
		using ClearRectType = VkClearRect;
		using ClearValueType = VkClearValue;
		using CommandBufferLevelType = VkCommandBufferLevel;
		using CommandBufferType = VkCommandBuffer;
		using CommandBufferUsageFlagsType = VkCommandBufferUsageFlags;
		using DeviceSizeType = VkDeviceSize;
		using ImageBlitType = VkImageBlit;
		using ImageCopyType = VkImageCopy;
		using ImageLayoutType = VkImageLayout;
		using ImageResolveType = VkImageResolve;
		using IndexTypeType = VkIndexType;
		using PipelineStagFlagBitsType = VkPipelineStageFlagBits;
		using QueryControlFlagsType = VkQueryControlFlags;
		using Rect2DType = VkRect2D;
		using ResultType = VkResult;
		using SubpassContentsType = VkSubpassContents;
		using ViewportType = VkViewport;

		using BufferMemoryBarrierType = vkb::BufferMemoryBarrier;
		using ColorBlendStateType = vkb::ColorBlendState;
		using DepthStencilStateType = vkb::DepthStencilState;
		using FramebufferType = vkb::Framebuffer;
		using ImageMemoryBarrierType = vkb::ImageMemoryBarrier;
		using ImageType = vkb::Image;
		using ImageViewType = vkb::ImageView;
		using InputAssemblyStateType = vkb::InputAssemblyState;
		using LoadStoreInfoType = vkb::LoadStoreInfo;
		using MultisampleStateType = vkb::MultisampleState;
		using PipelineLayoutType = vkb::PipelineLayout;
		using QueryPoolType = vkb::QueryPool;
		using RasterizationStateType = vkb::RasterizationState;
		using RenderPassType = vkb::RenderPass;
		using RenderTargetType = vkb::RenderTarget;
		using SamplerType = vkb::Sampler;
		using VertexInputStateType = vkb::VertexInputState;
		using ViewportStateType = vkb::ViewportState;

	public:
		CommandBuffer(vkb::CommandPool &command_pool, CommandBufferLevelType level);
		~CommandBuffer();
		CommandBuffer(CommandBuffer &&other) = default;
		CommandBuffer &operator=(CommandBuffer &&) = default;
		CommandBuffer(CommandBuffer const &) = delete;
		CommandBuffer &operator=(CommandBuffer const &) = delete;

		/**
		 * @brief Sets the command buffer so that it is ready for recording
		 *        If it is a secondary command buffer, a pointer to the
		 *        primary command buffer it inherits from must be provided
		 * @param flags Usage behavior for the command buffer
		 * @param primary_cmd_buf (optional)
		 */
		void begin(CommandBufferUsageFlagsType flags, CommandBuffer *primary_cmd_buf = nullptr);

		/**
		 * @brief Sets the command buffer so that it is ready for recording
		 *        If it is a secondary command buffer, pointers to the
		 *        render pass and framebuffer as well as subpass index must be provided
		 * @param flags Usage behavior for the command buffer
		 * @param render_pass
		 * @param framebuffer
		 * @param subpass_index
		 */
		void begin(CommandBufferUsageFlagsType flags, const RenderPassType *render_pass, const FramebufferType *framebuffer, uint32_t subpass_index);

		void begin_query(QueryPoolType const &query_pool, uint32_t query, QueryControlFlagsType flags);
		void begin_render_pass(RenderTargetType const &render_target,
							   std::vector<LoadStoreInfoType> const &load_store_infos,
							   std::vector<ClearValueType> const &clear_values,
							   std::vector<std::unique_ptr<vkb::Subpass>> const &subpasses,
							   SubpassContentsType contents = VK_SUBPASS_CONTENTS_INLINE);
		void begin_render_pass(RenderTargetType const &render_target,
							   RenderPassType const &render_pass,
							   FramebufferType const &framebuffer,
							   std::vector<ClearValueType> const &clear_values,
							   SubpassContentsType contents = vk::SubpassContents::eInline);
		void bind_buffer(vkb::Buffer const &buffer, DeviceSizeType offset, DeviceSizeType range, uint32_t set, uint32_t binding, uint32_t array_element);
		void bind_image(ImageViewType const &image_view, SamplerType const &sampler, uint32_t set, uint32_t binding, uint32_t array_element);
		void bind_image(ImageViewType const &image_view, uint32_t set, uint32_t binding, uint32_t array_element);
		void bind_index_buffer(vkb::Buffer const &buffer, DeviceSizeType offset, IndexTypeType index_type);
		void bind_input(ImageViewType const &image_view, uint32_t set, uint32_t binding, uint32_t array_element);
		void bind_lighting(vkb::LightingState &lighting_state, uint32_t set, uint32_t binding);
		void bind_pipeline_layout(PipelineLayoutType &pipeline_layout);
		void bind_vertex_buffers(uint32_t first_binding,
								 std::vector<std::reference_wrapper<const vkb::Buffer>> const &buffers,
								 std::vector<DeviceSizeType> const &offsets);
		void blit_image(ImageType const &src_img, ImageType const &dst_img, std::vector<ImageBlitType> const &regions);
		void buffer_memory_barrier(vkb::Buffer const &buffer, DeviceSizeType offset, DeviceSizeType size, BufferMemoryBarrierType const &memory_barrier);
		void clear(ClearAttachmentType const &info, ClearRectType const &rect);
		void copy_buffer(vkb::Buffer const &src_buffer, vkb::Buffer const &dst_buffer, DeviceSizeType size);
		void copy_buffer_to_image(vkb::Buffer const &buffer, ImageType const &image, std::vector<BufferImageCopyType> const &regions);
		void copy_image(ImageType const &src_img, ImageType const &dst_img, std::vector<ImageCopyType> const &regions);
		void copy_image_to_buffer(ImageType const &image,
								  ImageLayoutType image_layout,
								  vkb::Buffer const &buffer,
								  std::vector<BufferImageCopyType> const &regions);
		void dispatch(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z);
		void dispatch_indirect(vkb::Buffer const &buffer, DeviceSizeType offset);
		void draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);
		void draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance);
		void draw_indexed_indirect(vkb::Buffer const &buffer, DeviceSizeType offset, uint32_t draw_count, uint32_t stride);
		void end();
		void end_query(QueryPoolType const &query_pool, uint32_t query);
		void end_render_pass();
		void execute_commands(vkb::CommandBuffer &secondary_command_buffer);
		void execute_commands(std::vector<std::shared_ptr<vkb::CommandBuffer>> &secondary_command_buffers);
		CommandBufferLevelType get_level() const;
		RenderPassType &get_render_pass(RenderTargetType const &render_target,
										std::vector<LoadStoreInfoType> const &load_store_infos,
										std::vector<std::unique_ptr<vkb::Subpass>> const &subpasses);
		void image_memory_barrier(ImageViewType const &image_view, ImageMemoryBarrierType const &memory_barrier) const;
		void image_memory_barrier(RenderTargetType &render_target, uint32_t view_index, ImageMemoryBarrierType const &memory_barrier) const;
		void next_subpass();

		/**
		 * @brief Records byte data into the command buffer to be pushed as push constants to each draw call
		 * @param values The byte data to store
		 */
		void push_constants(const std::vector<uint8_t> &values);
		template <typename T>
		void push_constants(const T &value);

		/**
		 * @brief Reset the command buffer to a state where it can be recorded to
		 * @param reset_mode How to reset the buffer, should match the one used by the pool to allocate it
		 */
		ResultType reset(vkb::CommandBufferResetMode reset_mode);

		void reset_query_pool(QueryPoolType const &query_pool, uint32_t first_query, uint32_t query_count);
		void resolve_image(ImageType const &src_img, ImageType const &dst_img, std::vector<ImageResolveType> const &regions);
		void set_blend_constants(std::array<float, 4> const &blend_constants);
		void set_color_blend_state(ColorBlendStateType const &state_info);
		void set_depth_bias(float depth_bias_constant_factor, float depth_bias_clamp, float depth_bias_slope_factor);
		void set_depth_bounds(float min_depth_bounds, float max_depth_bounds);
		void set_depth_stencil_state(DepthStencilStateType const &state_info);
		void set_input_assembly_state(InputAssemblyStateType const &state_info);
		void set_line_width(float line_width);
		void set_multisample_state(MultisampleStateType const &state_info);
		void set_rasterization_state(RasterizationStateType const &state_info);
		void set_scissor(uint32_t first_scissor, std::vector<Rect2DType> const &scissors);

		template <class T>
		void set_specialization_constant(uint32_t constant_id, T const &data);
		void set_specialization_constant(uint32_t constant_id, std::vector<uint8_t> const &data);

		void set_update_after_bind(bool update_after_bind_);
		void set_vertex_input_state(VertexInputStateType const &state_info);
		void set_viewport(uint32_t first_viewport, std::vector<ViewportType> const &viewports);
		void set_viewport_state(ViewportStateType const &state_info);
		void update_buffer(vkb::Buffer const &buffer, DeviceSizeType offset, std::vector<uint8_t> const &data);
		void write_timestamp(PipelineStagFlagBitsType pipeline_stage, QueryPoolType const &query_pool, uint32_t query);

	private:
		/**
		 * @brief Flushes the command buffer, pushing the new changes
		 */
		void flush(vk::PipelineBindPoint pipeline_bind_point);

		/**
		 * @brief Flush the push constant state
		 */
		void flush_push_constants();

		/**
		 * @brief Check that the render area is an optimal size by comparing to the render area granularity
		 */
		bool is_render_size_optimal(const vk::Extent2D &extent, const vk::Rect2D &render_area);

	private:
		void begin_impl(vk::CommandBufferUsageFlags flags, vkb::CommandBuffer *primary_cmd_buf);
		void begin_impl(vk::CommandBufferUsageFlags flags,
						vkb::RenderPass const *render_pass,
						vkb::Framebuffer const *framebuffer,
						uint32_t subpass_index);
		void begin_render_pass_impl(vkb::RenderTarget const &render_target,
									vkb::RenderPass const &render_pass,
									vkb::Framebuffer const &framebuffer,
									std::vector<vk::ClearValue> const &clear_values,
									vk::SubpassContents contents);
		void bind_vertex_buffers_impl(uint32_t first_binding,
									  std::vector<std::reference_wrapper<const vkb::Buffer>> const &buffers,
									  std::vector<vk::DeviceSize> const &offsets);
		void buffer_memory_barrier_impl(vkb::Buffer const &buffer,
										vk::DeviceSize offset,
										vk::DeviceSize size,
										vkb::BufferMemoryBarrier const &memory_barrier);
		void copy_buffer_impl(vkb::Buffer const &src_buffer, vkb::BufferCpp const &dst_buffer, vk::DeviceSize size);
		void execute_commands_impl(std::vector<std::shared_ptr<vkb::CommandBuffer<vkb::BindingType::Cpp>>> &secondary_command_buffers);
		void flush_impl(vkb::VulkanDevice &device, vk::PipelineBindPoint pipeline_bind_point);
		void flush_descriptor_state_impl(vk::PipelineBindPoint pipeline_bind_point);
		void flush_pipeline_state_impl(vkb::VulkanDevice &device, vk::PipelineBindPoint pipeline_bind_point);
		vkb::RenderPass &get_render_pass_impl(vkb::VulkanDevice &device,
													   vkb::RenderTarget const &render_target,
													   std::vector<vkb::LoadStoreInfo> const &load_store_infos,
													   std::vector<std::unique_ptr<vkb::Subpass>> const &subpasses);
		void image_memory_barrier_impl(vkb::ImageView const &image_view, vkb::ImageMemoryBarrier const &memory_barrier) const;
		vk::Result reset_impl(vkb::CommandBufferResetMode reset_mode);
	};
}
