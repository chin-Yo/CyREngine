#pragma once

#include <cstdio>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "error.h"

#include <volk.h>

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