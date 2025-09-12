#include "Framework/Core/CommandBuffer.hpp"
#include "Framework/Core/CommandPool.hpp"
#include "Framework/Core/QueryPool.hpp"
#include "Framework/Rendering/RenderTarget.hpp"
#include "Framework/Core/Buffer.hpp"
#include "Framework/Core/Sampler.hpp"
#include "Framework/Rendering/Subpass.hpp"
namespace vkb
{
    CommandBuffer::CommandBuffer(CommandPool &command_pool_, VkCommandBufferLevel level_)
        : VulkanResource<VkCommandBuffer>(VK_NULL_HANDLE, &command_pool_.get_device()),
          level(level_),
          command_pool(command_pool_),
          max_push_constants_size(command_pool_.get_device().get_gpu().get_properties().limits.maxPushConstantsSize)
    {
        VkCommandBufferAllocateInfo allocate_info{};
        allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocate_info.pNext = nullptr;
        allocate_info.commandPool = command_pool.get_handle();
        allocate_info.level = level;
        allocate_info.commandBufferCount = 1;

        VkCommandBuffer handle = VK_NULL_HANDLE;
        VkResult result = vkAllocateCommandBuffers(GetDevice().GetHandle(), &allocate_info, &handle);

        if (result != VK_SUCCESS)
        {
            // TODO: LOG: Failed to allocate command buffer, error code: [result]
            handle = VK_NULL_HANDLE; // 确保句柄为空
        }

        SetHandle(handle);
    }

    CommandBuffer::~CommandBuffer()
    {
        // 销毁命令缓冲区
        if (GetHandle() != VK_NULL_HANDLE)
        {
            vkFreeCommandBuffers(GetDevice().GetHandle(), command_pool.get_handle(), 1, &GetHandle());
        }
    }

    void CommandBuffer::begin(VkCommandBufferUsageFlags flags,
                              vkb::CommandBuffer *primary_cmd_buf)
    {
        begin_impl(flags, primary_cmd_buf);
    }

    void CommandBuffer::begin_impl(VkCommandBufferUsageFlags flags, vkb::CommandBuffer *primary_cmd_buf)
    {
        if (level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)
        {
            assert(primary_cmd_buf && "A primary command buffer pointer must be provided when calling begin from a secondary one");
            return begin_impl(flags, primary_cmd_buf->current_render_pass, primary_cmd_buf->current_framebuffer, primary_cmd_buf->pipeline_state.get_subpass_index());
        }
        else
        {
            return begin_impl(flags, nullptr, nullptr, 0);
        }
    }
    void CommandBuffer::begin(VkCommandBufferUsageFlags flags,
                              const vkb::RenderPass *render_pass,
                              const vkb::Framebuffer *framebuffer,
                              uint32_t subpass_index)
    {
        begin_impl(flags, render_pass, framebuffer, subpass_index);
    }

    void CommandBuffer::begin_impl(VkCommandBufferUsageFlags flags,
                                   const vkb::RenderPass *render_pass,
                                   const vkb::Framebuffer *framebuffer,
                                   uint32_t subpass_index)
    {
        pipeline_state.reset();
        resource_binding_state.reset();
        descriptor_set_layout_binding_state.clear();
        stored_push_constants.clear();

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.pNext = nullptr;
        begin_info.flags = flags;
        begin_info.pInheritanceInfo = nullptr; // 先初始化为 nullptr

        VkCommandBufferInheritanceInfo inheritance_info{};
        inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        inheritance_info.pNext = nullptr;

        if (level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)
        {
            assert((render_pass && framebuffer) && "Render pass and framebuffer must be provided when calling begin from a secondary one");

            current_render_pass = render_pass;
            current_framebuffer = framebuffer;

            inheritance_info.renderPass = current_render_pass->GetHandle();
            inheritance_info.framebuffer = current_framebuffer->get_handle();
            inheritance_info.subpass = subpass_index;

            begin_info.pInheritanceInfo = &inheritance_info;
        }

        VkResult result = vkBeginCommandBuffer(this->GetHandle(), &begin_info);

        if (result != VK_SUCCESS)
        {
            // TODO LOG Failed to begin command buffer
        }
    }
    void CommandBuffer::begin_query(QueryPool const &query_pool, uint32_t query, VkQueryControlFlags flags)
    {
        vkCmdBeginQuery(
            this->GetHandle(),
            query_pool.get_handle(),
            query,
            flags);
    }

    void CommandBuffer::begin_render_pass(RenderTarget const &render_target,
                                          std::vector<LoadStoreInfo> const &load_store_infos,
                                          std::vector<VkClearValue> const &clear_values,
                                          std::vector<std::unique_ptr<vkb::Subpass>> const &subpasses,
                                          VkSubpassContents contents)
    {
        // Reset state
        pipeline_state.reset();
        resource_binding_state.reset();
        descriptor_set_layout_binding_state.clear();

        auto &render_pass = get_render_pass(render_target, load_store_infos, subpasses);
        auto &framebuffer = this->GetDevice().get_resource_cache().request_framebuffer(render_target, render_pass);

        begin_render_pass(render_target, render_pass, framebuffer, clear_values, contents);
    }

    void CommandBuffer::begin_render_pass(RenderTarget const &render_target,
                                          RenderPass const &render_pass,
                                          Framebuffer const &framebuffer,
                                          std::vector<VkClearValue> const &clear_values,
                                          VkSubpassContents contents)
    {
        begin_render_pass_impl(render_target, render_pass, framebuffer, clear_values, contents);
    }

    void CommandBuffer::begin_render_pass_impl(RenderTarget const &render_target,
                                               RenderPass const &render_pass,
                                               Framebuffer const &framebuffer,
                                               std::vector<VkClearValue> const &clear_values,
                                               VkSubpassContents contents)
    {
        current_render_pass = &render_pass;
        current_framebuffer = &framebuffer;

        // Begin render pass using C API
        VkRenderPassBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        begin_info.renderPass = current_render_pass->GetHandle();
        begin_info.framebuffer = current_framebuffer->get_handle();
        begin_info.renderArea.extent = render_target.get_extent();
        begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        begin_info.pClearValues = clear_values.data();

        const auto &framebuffer_extent = current_framebuffer->get_extent();

        // Test render area optimization
        if (!is_render_size_optimal(framebuffer_extent, begin_info.renderArea))
        {
            // 手动比较宽度和高度
            bool framebuffer_changed =
                (framebuffer_extent.width != last_framebuffer_extent.width) ||
                (framebuffer_extent.height != last_framebuffer_extent.height);

            bool render_area_changed =
                (begin_info.renderArea.extent.width != last_render_area_extent.width) ||
                (begin_info.renderArea.extent.height != last_render_area_extent.height);

            if (framebuffer_changed || render_area_changed)
            {
                // TODO LOG("Render target extent is not an optimal size");
            }
            last_framebuffer_extent = framebuffer_extent;
            last_render_area_extent = begin_info.renderArea.extent;
        }

        vkCmdBeginRenderPass(this->GetHandle(), &begin_info, contents);

        // Update blend state attachments
        auto blend_state = pipeline_state.get_color_blend_state();
        blend_state.attachments.resize(current_render_pass->get_color_output_count(pipeline_state.get_subpass_index()));
        pipeline_state.set_color_blend_state(blend_state);
    }

    void CommandBuffer::bind_buffer(
        vkb::Buffer const &buffer, DeviceSizeType offset, DeviceSizeType range, uint32_t set, uint32_t binding, uint32_t array_element)
    {
        // 使用 C API 实现
        resource_binding_state.bind_buffer(
            buffer, // 获取底层 VkBuffer
            offset,
            range,
            set,
            binding,
            array_element);
    }

    void CommandBuffer::bind_image(
        vkb::ImageView const &image_view, vkb::Sampler const &sampler, uint32_t set, uint32_t binding, uint32_t array_element)
    {
        // 使用 C API 实现
        resource_binding_state.bind_image(
            image_view, // 获取底层 VkImageView
            sampler,    // 获取底层 VkSampler
            set,
            binding,
            array_element);
    }

    void CommandBuffer::bind_image(vkb::ImageView const &image_view, uint32_t set, uint32_t binding, uint32_t array_element)
    {
        // 使用 C API 实现
        resource_binding_state.bind_image(
            image_view, // 获取底层 VkImageView
            set,
            binding,
            array_element);
    }

    void CommandBuffer::bind_index_buffer(vkb::Buffer const &buffer, VkDeviceSize offset, VkIndexType index_type)
    {
        vkCmdBindIndexBuffer(GetHandle(), buffer.GetHandle(), offset, index_type);
    }

    void CommandBuffer::bind_input(vkb::ImageView const &image_view, uint32_t set, uint32_t binding, uint32_t array_element)
    {
        resource_binding_state.bind_input(image_view, set, binding, array_element);
    }

    void CommandBuffer::bind_lighting(vkb::LightingState &lighting_state, uint32_t set, uint32_t binding)
    {
        bind_buffer(lighting_state.light_buffer.get_buffer(), lighting_state.light_buffer.get_offset(), lighting_state.light_buffer.get_size(), set, binding, 0);

        set_specialization_constant(0, to_u32(lighting_state.directional_lights.size()));
        set_specialization_constant(1, to_u32(lighting_state.point_lights.size()));
        set_specialization_constant(2, to_u32(lighting_state.spot_lights.size()));
    }

    void CommandBuffer::bind_pipeline_layout(vkb::PipelineLayout &pipeline_layout)
    {
        pipeline_state.set_pipeline_layout(pipeline_layout);
    }

    void CommandBuffer::bind_vertex_buffers(uint32_t first_binding,
                                                   std::vector<std::reference_wrapper<const vkb::Buffer>> const &buffers,
                                                   std::vector<VkDeviceSize> const &offsets)
    {
        if (buffers.size() != offsets.size())
        {
            return;
        }

        std::vector<VkBuffer> buffer_handles;
        buffer_handles.reserve(buffers.size());
        std::transform(buffers.begin(), buffers.end(), std::back_inserter(buffer_handles),
                       [](auto const &buffer_wrapper)
                       {
                           return buffer_wrapper.get().GetHandle();
                       });

        vkCmdBindVertexBuffers(GetHandle(),
                               first_binding,
                               static_cast<uint32_t>(buffers.size()),
                               buffer_handles.data(),
                               offsets.data());
    }

    void CommandBuffer::blit_image(vkb::Image const &src_img, vkb::Image const &dst_img, std::vector<VkImageBlit> const &regions)
    {
        vkCmdBlitImage(GetHandle(),
                       src_img.GetHandle(),
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       dst_img.GetHandle(),
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       static_cast<uint32_t>(regions.size()),
                       regions.data(),
                       VK_FILTER_NEAREST);
    }

    void CommandBuffer::buffer_memory_barrier(vkb::Buffer const &buffer,
                                                     DeviceSizeType offset,
                                                     DeviceSizeType size,
                                                     BufferMemoryBarrierType const &memory_barrier)
    {
        buffer_memory_barrier_impl(buffer, offset, size, memory_barrier);
    }

    void CommandBuffer::buffer_memory_barrier_impl(vkb::Buffer const &buffer,
                                                          DeviceSizeType offset,
                                                          DeviceSizeType size,
                                                          vkb::BufferMemoryBarrier const &memory_barrier)
    {
        VkBufferMemoryBarrier buffer_memory_barrier;
        buffer_memory_barrier.srcAccessMask = memory_barrier.src_access_mask;
        buffer_memory_barrier.dstAccessMask = memory_barrier.dst_access_mask;
        buffer_memory_barrier.buffer = buffer.GetHandle();
        buffer_memory_barrier.offset = offset;
        buffer_memory_barrier.size = size;

        vkCmdPipelineBarrier(GetHandle(), memory_barrier.src_stage_mask, memory_barrier.dst_stage_mask, 0,
                             0, nullptr,
                             1, &buffer_memory_barrier,
                             0, nullptr);
    }

    void CommandBuffer::clear(ClearAttachmentType const &attachment, ClearRectType const &rect)
    {
        vkCmdClearAttachments(GetHandle(), 1, &attachment, 1, &rect);
    }

    void CommandBuffer::copy_buffer(vkb::Buffer const &src_buffer,
                                           vkb::Buffer const &dst_buffer,
                                           DeviceSizeType size)
    {
        copy_buffer_impl(src_buffer, dst_buffer, size);
    }

    void
    CommandBuffer::copy_buffer_impl(vkb::Buffer const &src_buffer, vkb::Buffer const &dst_buffer, VkDeviceSize size)
    {
        VkBufferCopy copy_region{};
        copy_region.srcOffset = 0;
        copy_region.dstOffset = 0;
        copy_region.size = size;
        vkCmdCopyBuffer(GetHandle(), src_buffer.GetHandle(), dst_buffer.GetHandle(), 1, &copy_region);
    }

    void CommandBuffer::copy_buffer_to_image(vkb::Buffer const &buffer,
                                                    ImageType const &image,
                                                    std::vector<BufferImageCopyType> const &regions)
    {
        if (regions.empty())
        {
            return;
        }
        vkCmdCopyBufferToImage(GetHandle(),
                               buffer.GetHandle(),
                               image.GetHandle(),
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               static_cast<uint32_t>(regions.size()),
                               regions.data());
    }

    /**
     * @brief 将源图像的内容复制到目标图像
     * @param src_img 源图像对象
     * @param dst_img 目标图像对象
     * @param regions 定义了要复制的图像区域的集合
     */
    void vkb::CommandBuffer::copy_image(const vkb::Image &src_img, const vkb::Image &dst_img, const std::vector<VkImageCopy> &regions)
    {
        // 使用Vulkan C API函数 vkCmdCopyImage
        vkCmdCopyImage(
            GetHandle(),                           // 当前指令缓冲区的句柄 (VkCommandBuffer)
            src_img.GetHandle(),                   // 源图像的句柄 (VkImage)
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,  // 源图像布局
            dst_img.GetHandle(),                   // 目标图像的句柄 (VkImage)
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,  // 目标图像布局
            static_cast<uint32_t>(regions.size()), // 要复制的区域数量
            regions.data()                         // 指向区域数组的指针
        );
    }

    /**
     * @brief 将图像内容复制到缓冲区
     * @param image 源图像对象
     * @param image_layout 源图像的当前布局
     * @param buffer 目标缓冲区对象
     * @param regions 定义了要复制的区域的集合
     */
    void vkb::CommandBuffer::copy_image_to_buffer(const vkb::Image &image,
                                                         VkImageLayout image_layout,
                                                         const vkb::Buffer &buffer,
                                                         const std::vector<VkBufferImageCopy> &regions)
    {
        vkCmdCopyImageToBuffer(
            GetHandle(),                           // 当前指令缓冲区的句柄 (VkCommandBuffer)
            image.GetHandle(),                     // 源图像的句柄 (VkImage)
            image_layout,                          // 源图像布局
            buffer.GetHandle(),                    // 目标缓冲区的句柄 (VkBuffer)
            static_cast<uint32_t>(regions.size()), // 要复制的区域数量
            regions.data()                         // 指向区域数组的指针
        );
    }

    /**
     * @brief 在计算管线中分发计算任务
     * @param group_count_x X维度的工作组数量
     * @param group_count_y Y维度的工作组数量
     * @param group_count_z Z维度的工作组数量
     */
    void vkb::CommandBuffer::dispatch(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z)
    {
        // flush 函数的作用是确保在执行dispatch前，所有绑定的状态（如描述符集）都已更新
        flush(VK_PIPELINE_BIND_POINT_COMPUTE);

        // 使用Vulkan C API函数 vkCmdDispatch
        vkCmdDispatch(GetHandle(), group_count_x, group_count_y, group_count_z);
    }

    /**
     * @brief 间接分发计算任务，参数从缓冲区中读取
     * @param buffer 包含分发参数的缓冲区 (VkBuffer)
     * @param offset 参数在缓冲区中的偏移量
     */
    void vkb::CommandBuffer::dispatch_indirect(const vkb::Buffer &buffer, VkDeviceSize offset)
    {
        flush(VK_PIPELINE_BIND_POINT_COMPUTE);

        vkCmdDispatchIndirect(GetHandle(), buffer.GetHandle(), offset);
    }

    void CommandBuffer::draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
    {
        flush(VK_PIPELINE_BIND_POINT_GRAPHICS);
        vkCmdDraw(this->GetHandle(), vertex_count, instance_count, first_vertex, first_instance);
    }

    void CommandBuffer::draw_indexed(
        uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance)
    {
        flush(VK_PIPELINE_BIND_POINT_GRAPHICS);
        vkCmdDrawIndexed(this->GetHandle(), index_count, instance_count, first_index, vertex_offset, first_instance);
    }

    void CommandBuffer::draw_indexed_indirect(vkb::Buffer const &buffer, VkDeviceSize offset, uint32_t draw_count, uint32_t stride)
    {
        flush(VK_PIPELINE_BIND_POINT_GRAPHICS);
        vkCmdDrawIndexedIndirect(this->GetHandle(), buffer.GetHandle(), offset, draw_count, stride);
    }

    void CommandBuffer::end()
    {
        if (vkEndCommandBuffer(this->GetHandle()) != VK_SUCCESS)
        {
            // TODO  LOG
        }
    }

    void CommandBuffer::end_query(QueryPoolType const &query_pool, uint32_t query)
    {
        // TODO: Assuming query_pool.get_handle() returns a valid VkQueryPool handle
        vkCmdEndQuery(this->GetHandle(), query_pool.get_handle(), query);
    }

    void CommandBuffer::end_render_pass()
    {
        vkCmdEndRenderPass(this->GetHandle());
    }

    VkCommandBufferLevel CommandBuffer::get_level() const
    {
        // Assuming the member variable 'level' is of type VkCommandBufferLevel
        return level;
    }

    void CommandBuffer::execute_commands(vkb::CommandBuffer &secondary_command_buffer)
    {
        // vkCmdExecuteCommands expects a pointer to an array of command buffers
        // TODO: Assuming get_resource() returns a valid VkCommandBuffer handle
        VkCommandBuffer secondary_cmd_buffer_handle = secondary_command_buffer.GetHandle();
        vkCmdExecuteCommands(this->GetHandle(), 1, &secondary_cmd_buffer_handle);
    }

    void CommandBuffer::execute_commands(std::vector<std::shared_ptr<vkb::CommandBuffer>> &secondary_command_buffers)
    {
        execute_commands_impl(secondary_command_buffers);
    }
    void CommandBuffer::execute_commands_impl(std::vector<std::shared_ptr<vkb::CommandBuffer>> &secondary_command_buffers)
    {
        if (secondary_command_buffers.empty())
        {
            return;
        }

        std::vector<VkCommandBuffer> sec_cmd_buf_handles;
        sec_cmd_buf_handles.reserve(secondary_command_buffers.size());

        std::transform(secondary_command_buffers.begin(),
                       secondary_command_buffers.end(),
                       std::back_inserter(sec_cmd_buf_handles),
                       [](const std::shared_ptr<vkb::CommandBuffer> &sec_cmd_buf)
                       {
                           // TODO: Assuming get_resource() returns a valid VkCommandBuffer handle
                           return sec_cmd_buf->GetHandle();
                       });

        vkCmdExecuteCommands(this->GetHandle(),
                             static_cast<uint32_t>(sec_cmd_buf_handles.size()),
                             sec_cmd_buf_handles.data());
    }

    typename vkb::CommandBuffer::RenderPassType &
    CommandBuffer::get_render_pass(RenderTargetType const &render_target,
                                   std::vector<LoadStoreInfoType> const &load_store_infos,
                                   std::vector<std::unique_ptr<vkb::Subpass>> const &subpasses)
    {

        return get_render_pass_impl(this->GetDevice(), render_target, load_store_infos, subpasses);
    }

    vkb::RenderPass &
    CommandBuffer::get_render_pass_impl(vkb::VulkanDevice &device,
                                        vkb::RenderTarget const &render_target,
                                        std::vector<vkb::LoadStoreInfo> const &load_store_infos,
                                        std::vector<std::unique_ptr<vkb::Subpass>> const &subpasses)
    {
        // Create render pass
        assert(subpasses.size() > 0 && "Cannot create a render pass without any subpass");

        std::vector<vkb::SubpassInfo> subpass_infos(subpasses.size());
        auto subpass_info_it = subpass_infos.begin();
        for (auto &subpass : subpasses)
        {
            subpass_info_it->input_attachments = subpass->get_input_attachments();
            subpass_info_it->output_attachments = subpass->get_output_attachments();
            subpass_info_it->color_resolve_attachments = subpass->get_color_resolve_attachments();
            subpass_info_it->disable_depth_stencil_attachment = subpass->get_disable_depth_stencil_attachment();
            subpass_info_it->depth_stencil_resolve_mode = subpass->get_depth_stencil_resolve_mode();
            subpass_info_it->depth_stencil_resolve_attachment = subpass->get_depth_stencil_resolve_attachment();
            subpass_info_it->debug_name = subpass->get_debug_name();

            ++subpass_info_it;
        }

        return device.get_resource_cache().request_render_pass(render_target.get_attachments(), load_store_infos, subpass_infos);
    }
    void CommandBuffer::image_memory_barrier(RenderTargetType &render_target, uint32_t view_index, ImageMemoryBarrierType const &memory_barrier) const
    {
        auto const &image_view = render_target.get_views()[view_index];

        image_memory_barrier_impl(image_view, memory_barrier);

        render_target.set_layout(view_index, memory_barrier.new_layout);
    }

    void CommandBuffer::image_memory_barrier(ImageViewType const &image_view, ImageMemoryBarrierType const &memory_barrier) const
    {
        image_memory_barrier_impl(image_view, memory_barrier);
    }

    void CommandBuffer::image_memory_barrier_impl(vkb::ImageView const &image_view,
                                                         vkb::ImageMemoryBarrier const &memory_barrier) const
    {

        VkImageSubresourceRange subresource_range = image_view.get_subresource_range(); // 假设已从 image_view 获取
        VkFormat format = image_view.get_format();                                      // 假设已从 image_view 获取

        if (is_depth_only_format(format))
        {
            subresource_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        else if (is_depth_stencil_format(format))
        {
            subresource_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        VkImageMemoryBarrier image_memory_barrier{};
        image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        image_memory_barrier.srcAccessMask = memory_barrier.src_access_mask;
        image_memory_barrier.dstAccessMask = memory_barrier.dst_access_mask;
        image_memory_barrier.oldLayout = memory_barrier.old_layout;
        image_memory_barrier.newLayout = memory_barrier.new_layout;
        image_memory_barrier.srcQueueFamilyIndex = memory_barrier.src_queue_family;
        image_memory_barrier.dstQueueFamilyIndex = memory_barrier.dst_queue_family;
        image_memory_barrier.image = image_view.get_image().GetHandle();
        image_memory_barrier.subresourceRange = subresource_range;

        vkCmdPipelineBarrier(
            this->GetHandle(), // VkCommandBuffer
            memory_barrier.src_stage_mask,
            memory_barrier.dst_stage_mask,
            0,       // dependencyFlags
            0,       // memoryBarrierCount
            nullptr, // pMemoryBarriers
            0,       // bufferMemoryBarrierCount
            nullptr, // pBufferMemoryBarriers
            1,       // imageMemoryBarrierCount
            &image_memory_barrier);
    }

    void CommandBuffer::next_subpass()
    {
        // Increment subpass index
        pipeline_state.set_subpass_index(pipeline_state.get_subpass_index() + 1);

        // Update blend state attachments
        auto blend_state = pipeline_state.get_color_blend_state();
        blend_state.attachments.resize(current_render_pass->get_color_output_count(pipeline_state.get_subpass_index()));
        pipeline_state.set_color_blend_state(blend_state);

        // Reset descriptor sets
        resource_binding_state.reset();
        descriptor_set_layout_binding_state.clear();

        // Clear stored push constants
        stored_push_constants.clear();

        vkCmdNextSubpass(
            this->GetHandle(), // VkCommandBuffer
            VK_SUBPASS_CONTENTS_INLINE);
    }

    void CommandBuffer::push_constants(const std::vector<uint8_t> &values)
    {
        uint32_t push_constant_size = to_u32(stored_push_constants.size() + values.size());

        if (push_constant_size > max_push_constants_size)
        {
            LOGE("Push constant limit of {} exceeded (pushing {} bytes for a total of {} bytes)", max_push_constants_size, values.size(), push_constant_size);
            // TODO throw std::runtime_error("Push constant limit exceeded.");
        }
        else
        {
            stored_push_constants.insert(stored_push_constants.end(), values.begin(), values.end());
        }
    }
}