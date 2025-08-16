#pragma once

#include <cstdio>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "error.h"

#include <volk.h>

#include <type_traits>
#include <vk_mem_alloc.h>


template <typename T>
constexpr VkObjectType GetVkObjectType();

#define DEFINE_VK_OBJECT_TYPE(Type, ObjectType) \
template <>                                 \
constexpr VkObjectType GetVkObjectType<Type>() { return ObjectType; }

DEFINE_VK_OBJECT_TYPE(VkInstance, VK_OBJECT_TYPE_INSTANCE)
DEFINE_VK_OBJECT_TYPE(VkPhysicalDevice, VK_OBJECT_TYPE_PHYSICAL_DEVICE)
DEFINE_VK_OBJECT_TYPE(VkDevice, VK_OBJECT_TYPE_DEVICE)
DEFINE_VK_OBJECT_TYPE(VkQueue, VK_OBJECT_TYPE_QUEUE)
DEFINE_VK_OBJECT_TYPE(VkSemaphore, VK_OBJECT_TYPE_SEMAPHORE)
DEFINE_VK_OBJECT_TYPE(VkCommandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER)
DEFINE_VK_OBJECT_TYPE(VkFence, VK_OBJECT_TYPE_FENCE)
DEFINE_VK_OBJECT_TYPE(VkDeviceMemory, VK_OBJECT_TYPE_DEVICE_MEMORY)
DEFINE_VK_OBJECT_TYPE(VkBuffer, VK_OBJECT_TYPE_BUFFER)
DEFINE_VK_OBJECT_TYPE(VkImage, VK_OBJECT_TYPE_IMAGE)
DEFINE_VK_OBJECT_TYPE(VkEvent, VK_OBJECT_TYPE_EVENT)
DEFINE_VK_OBJECT_TYPE(VkQueryPool, VK_OBJECT_TYPE_QUERY_POOL)
DEFINE_VK_OBJECT_TYPE(VkBufferView, VK_OBJECT_TYPE_BUFFER_VIEW)
DEFINE_VK_OBJECT_TYPE(VkImageView, VK_OBJECT_TYPE_IMAGE_VIEW)
DEFINE_VK_OBJECT_TYPE(VkShaderModule, VK_OBJECT_TYPE_SHADER_MODULE)
DEFINE_VK_OBJECT_TYPE(VkPipelineCache, VK_OBJECT_TYPE_PIPELINE_CACHE)
DEFINE_VK_OBJECT_TYPE(VkPipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT)
DEFINE_VK_OBJECT_TYPE(VkRenderPass, VK_OBJECT_TYPE_RENDER_PASS)
DEFINE_VK_OBJECT_TYPE(VkPipeline, VK_OBJECT_TYPE_PIPELINE)
DEFINE_VK_OBJECT_TYPE(VkDescriptorSetLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT)
DEFINE_VK_OBJECT_TYPE(VkSampler, VK_OBJECT_TYPE_SAMPLER)
DEFINE_VK_OBJECT_TYPE(VkDescriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL)
DEFINE_VK_OBJECT_TYPE(VkDescriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET)
DEFINE_VK_OBJECT_TYPE(VkFramebuffer, VK_OBJECT_TYPE_FRAMEBUFFER)
DEFINE_VK_OBJECT_TYPE(VkCommandPool, VK_OBJECT_TYPE_COMMAND_POOL)
#define VK_FLAGS_NONE 0 // Custom define for better code readability

#define DEFAULT_FENCE_TIMEOUT 100000000000 // Default fence timeout in nanoseconds

template <class T>
using ShaderStageMap = std::map<VkShaderStageFlagBits, T>;

template <class T>
using BindingMap = std::map<uint32_t, std::map<uint32_t, T>>;

namespace vkb
{

	/**
	 * @brief Helper function to determine if a Vulkan format is depth only.
	 * @param format Vulkan format to check.
	 * @return True if format is a depth only, false otherwise.
	 */
	bool is_depth_only_format(VkFormat format);

	/**
	 * @brief Helper function to determine if a Vulkan format is depth with stencil.
	 * @param format Vulkan format to check.
	 * @return True if format is a depth with stencil, false otherwise.
	 */
	bool is_depth_stencil_format(VkFormat format);

	/**
	 * @brief Helper function to determine if a Vulkan format is depth.
	 * @param format Vulkan format to check.
	 * @return True if format is a depth, false otherwise.
	 */
	bool is_depth_format(VkFormat format);
}