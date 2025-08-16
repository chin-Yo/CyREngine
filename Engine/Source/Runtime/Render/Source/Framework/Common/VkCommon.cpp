#include "Framework/Common/VkCommon.hpp"

namespace vkb
{
	bool is_depth_only_format(VkFormat format)
	{
		return format == VK_FORMAT_D16_UNORM ||
			   format == VK_FORMAT_D32_SFLOAT;
	}

	bool is_depth_stencil_format(VkFormat format)
	{
		return format == VK_FORMAT_D16_UNORM_S8_UINT ||
			   format == VK_FORMAT_D24_UNORM_S8_UINT ||
			   format == VK_FORMAT_D32_SFLOAT_S8_UINT;
	}

	bool is_depth_format(VkFormat format)
	{
		return is_depth_only_format(format) || is_depth_stencil_format(format);
	}

	std::vector<VkImageCompressionFixedRateFlagBitsEXT> fixed_rate_compression_flags_to_vector(VkImageCompressionFixedRateFlagsEXT flags)
	{
		const std::vector<VkImageCompressionFixedRateFlagBitsEXT> all_flags = {VK_IMAGE_COMPRESSION_FIXED_RATE_1BPC_BIT_EXT, VK_IMAGE_COMPRESSION_FIXED_RATE_2BPC_BIT_EXT,
																			   VK_IMAGE_COMPRESSION_FIXED_RATE_3BPC_BIT_EXT, VK_IMAGE_COMPRESSION_FIXED_RATE_4BPC_BIT_EXT,
																			   VK_IMAGE_COMPRESSION_FIXED_RATE_5BPC_BIT_EXT, VK_IMAGE_COMPRESSION_FIXED_RATE_6BPC_BIT_EXT,
																			   VK_IMAGE_COMPRESSION_FIXED_RATE_7BPC_BIT_EXT, VK_IMAGE_COMPRESSION_FIXED_RATE_8BPC_BIT_EXT,
																			   VK_IMAGE_COMPRESSION_FIXED_RATE_9BPC_BIT_EXT, VK_IMAGE_COMPRESSION_FIXED_RATE_10BPC_BIT_EXT,
																			   VK_IMAGE_COMPRESSION_FIXED_RATE_11BPC_BIT_EXT, VK_IMAGE_COMPRESSION_FIXED_RATE_12BPC_BIT_EXT,
																			   VK_IMAGE_COMPRESSION_FIXED_RATE_13BPC_BIT_EXT, VK_IMAGE_COMPRESSION_FIXED_RATE_14BPC_BIT_EXT,
																			   VK_IMAGE_COMPRESSION_FIXED_RATE_15BPC_BIT_EXT, VK_IMAGE_COMPRESSION_FIXED_RATE_16BPC_BIT_EXT,
																			   VK_IMAGE_COMPRESSION_FIXED_RATE_17BPC_BIT_EXT, VK_IMAGE_COMPRESSION_FIXED_RATE_18BPC_BIT_EXT,
																			   VK_IMAGE_COMPRESSION_FIXED_RATE_19BPC_BIT_EXT, VK_IMAGE_COMPRESSION_FIXED_RATE_20BPC_BIT_EXT,
																			   VK_IMAGE_COMPRESSION_FIXED_RATE_21BPC_BIT_EXT, VK_IMAGE_COMPRESSION_FIXED_RATE_22BPC_BIT_EXT,
																			   VK_IMAGE_COMPRESSION_FIXED_RATE_23BPC_BIT_EXT, VK_IMAGE_COMPRESSION_FIXED_RATE_24BPC_BIT_EXT};

		std::vector<VkImageCompressionFixedRateFlagBitsEXT> flags_vector;

		for (size_t i = 0; i < all_flags.size(); i++)
		{
			if (all_flags[i] & flags)
			{
				flags_vector.push_back(all_flags[i]);
			}
		}

		return flags_vector;
	}

	VkImageCompressionPropertiesEXT query_supported_fixed_rate_compression(VkPhysicalDevice gpu, const VkImageCreateInfo &create_info)
	{
		VkImageCompressionPropertiesEXT supported_compression_properties{VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_PROPERTIES_EXT};

		VkImageCompressionControlEXT compression_control{VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_CONTROL_EXT};
		compression_control.flags = VK_IMAGE_COMPRESSION_FIXED_RATE_DEFAULT_EXT;

		VkPhysicalDeviceImageFormatInfo2 image_format_info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2};
		image_format_info.format = create_info.format;
		image_format_info.type = create_info.imageType;
		image_format_info.tiling = create_info.tiling;
		image_format_info.usage = create_info.usage;
		image_format_info.pNext = &compression_control;

		VkImageFormatProperties2 image_format_properties{VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2};
		image_format_properties.pNext = &supported_compression_properties;

		vkGetPhysicalDeviceImageFormatProperties2KHR(gpu, &image_format_info, &image_format_properties);

		return supported_compression_properties;
	}

	VkImageCompressionPropertiesEXT query_applied_compression(VkDevice device, VkImage image)
	{
		VkImageSubresource2EXT image_subresource{VK_STRUCTURE_TYPE_IMAGE_SUBRESOURCE_2_KHR};
		image_subresource.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_subresource.imageSubresource.mipLevel = 0;
		image_subresource.imageSubresource.arrayLayer = 0;

		VkImageCompressionPropertiesEXT compression_properties{VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_PROPERTIES_EXT};
		VkSubresourceLayout2EXT subresource_layout{VK_STRUCTURE_TYPE_SUBRESOURCE_LAYOUT_2_KHR};
		subresource_layout.pNext = &compression_properties;

		vkGetImageSubresourceLayout2EXT(device, image, &image_subresource, &subresource_layout);

		return compression_properties;
	}
}