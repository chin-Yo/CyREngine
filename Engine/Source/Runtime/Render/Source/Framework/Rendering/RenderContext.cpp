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

#include "Framework/Rendering/RenderContext.hpp"
#include "Framework/Core/VulkanDevice.hpp"
#include "Framework/Common/VkError.hpp"
#include "Framework/Platform/Window.hpp"
#include "Framework/Rendering/RenderFrame.hpp"

namespace vkb
{
    VkFormat RenderContext::DEFAULT_VK_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;

    RenderContext::RenderContext(VulkanDevice &device,
                                 VkSurfaceKHR surface,
                                 const Window &window,
                                 VkPresentModeKHR present_mode,
                                 const std::vector<VkPresentModeKHR> &present_mode_priority_list,
                                 const std::vector<VkSurfaceFormatKHR> &surface_format_priority_list) : device{device}, window{window}, queue{device.get_suitable_graphics_queue()}, surface_extent{window.get_extent().width, window.get_extent().height}
    {
        if (surface != VK_NULL_HANDLE)
        {
            VkSurfaceCapabilitiesKHR surface_properties;
            VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.get_gpu().get_handle(),
                                                                      surface,
                                                                      &surface_properties));
            // The fact that VkSurfaceCapabilitiesKHR::currentExtent.width equals 0xFFFFFFFF
            // (which is 0xFFFFFFFF or UINT32_MAX) is a special value,
            // indicating that the size of the swap chain is determined by
            // the application rather than being strictly constrained by the window system.
            if (surface_properties.currentExtent.width == 0xFFFFFFFF)
            {
                swapchain = std::make_unique<Swapchain>(device, surface, present_mode, present_mode_priority_list, surface_format_priority_list, surface_extent);
            }
            else
            {
                swapchain = std::make_unique<Swapchain>(device, surface, present_mode, present_mode_priority_list, surface_format_priority_list);
            }
        }
    }

    void RenderContext::prepare(size_t thread_count, RenderTarget::CreateFunc create_render_target_func)
    {
        device.wait_idle();

        if (swapchain)
        {
            surface_extent = swapchain->get_extent();

            VkExtent3D extent{surface_extent.width, surface_extent.height, 1};

            for (auto &image_handle : swapchain->get_images())
            {
                auto swapchain_image = Image{
                    device, image_handle,
                    extent,
                    swapchain->get_format(),
                    swapchain->get_usage()};
                auto render_target = create_render_target_func(std::move(swapchain_image));
                frames.emplace_back(std::make_unique<vkb::RenderFrame>(device, std::move(render_target), thread_count));
            }
        }
        else
        {
            // Otherwise, create a single RenderFrame
            swapchain = nullptr;

            auto color_image = Image{device,
                                     VkExtent3D{surface_extent.width, surface_extent.height, 1},
                                     DEFAULT_VK_FORMAT, // We can use any format here that we like
                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                     VMA_MEMORY_USAGE_GPU_ONLY};

            auto render_target = create_render_target_func(std::move(color_image));
            frames.emplace_back(std::make_unique<vkb::RenderFrame>(device, std::move(render_target), thread_count));
        }

        this->create_render_target_func = create_render_target_func;
        this->thread_count = thread_count;
        this->prepared = true;
    }
    VkFormat RenderContext::get_format() const
    {
        VkFormat format = DEFAULT_VK_FORMAT;

        if (swapchain)
        {
            format = swapchain->get_format();
        }

        return format;
    }

    void RenderContext::update_swapchain(const VkExtent2D &extent)
    {
        if (!swapchain)
        {
            LOGW("Can't update the swapchains extent. No swapchain, offscreen rendering detected, skipping.");
            return;
        }

        device.get_resource_cache().clear_framebuffers();

        swapchain = std::make_unique<Swapchain>(*swapchain, extent);

        recreate();
    }

    void RenderContext::update_swapchain(const uint32_t image_count)
    {
        if (!swapchain)
        {
            LOGW("Can't update the swapchains image count. No swapchain, offscreen rendering detected, skipping.");
            return;
        }

        device.get_resource_cache().clear_framebuffers();

        device.wait_idle();

        swapchain = std::make_unique<Swapchain>(*swapchain, image_count);

        recreate();
    }

    void RenderContext::update_swapchain(const std::set<VkImageUsageFlagBits> &image_usage_flags)
    {
        if (!swapchain)
        {
            LOGW("Can't update the swapchains image usage. No swapchain, offscreen rendering detected, skipping.");
            return;
        }

        device.get_resource_cache().clear_framebuffers();

        swapchain = std::make_unique<Swapchain>(*swapchain, image_usage_flags);

        recreate();
    }

    void RenderContext::update_swapchain(const VkExtent2D &extent, const VkSurfaceTransformFlagBitsKHR transform)
    {
        if (!swapchain)
        {
            LOGW("Can't update the swapchains extent and surface transform. No swapchain, offscreen rendering detected, skipping.");
            return;
        }

        device.get_resource_cache().clear_framebuffers();

        auto width = extent.width;
        auto height = extent.height;
        if (transform == VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR || transform == VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
        {
            // Pre-rotation: always use native orientation i.e. if rotated, use width and height of identity transform
            std::swap(width, height);
        }

        swapchain = std::make_unique<Swapchain>(*swapchain, VkExtent2D{width, height}, transform);

        // Save the preTransform attribute for future rotations
        pre_transform = transform;

        recreate();
    }

    void RenderContext::update_swapchain(const VkImageCompressionFlagsEXT compression, const VkImageCompressionFixedRateFlagsEXT compression_fixed_rate)
    {
        if (!swapchain)
        {
            LOGW("Can't update the swapchains compression. No swapchain, offscreen rendering detected, skipping.");
            return;
        }

        device.get_resource_cache().clear_framebuffers();

        swapchain = std::make_unique<Swapchain>(*swapchain, compression, compression_fixed_rate);

        recreate();
    }

    void RenderContext::recreate()
    {
        LOGI("Recreated swapchain");

        VkExtent2D swapchain_extent = swapchain->get_extent();
        VkExtent3D extent{swapchain_extent.width, swapchain_extent.height, 1};

        auto frame_it = frames.begin();

        for (auto &image_handle : swapchain->get_images())
        {
            Image swapchain_image{device, image_handle,
                                  extent,
                                  swapchain->get_format(),
                                  swapchain->get_usage()};

            auto render_target = create_render_target_func(std::move(swapchain_image));

            if (frame_it != frames.end())
            {
                (*frame_it)->update_render_target(std::move(render_target));
            }
            else
            {
                // Create a new frame if the new swapchain has more images than current frames
                frames.emplace_back(std::make_unique<vkb::RenderFrame>(device, std::move(render_target), thread_count));
            }

            ++frame_it;
        }

        device.get_resource_cache().clear_framebuffers();
    }
}
