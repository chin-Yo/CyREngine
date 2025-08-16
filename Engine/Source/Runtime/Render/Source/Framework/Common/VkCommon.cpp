#include "Framework/Common/VkCommon.hpp"

#include "Logging/Logger.hpp"

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


    VkFormat get_suitable_depth_format(VkPhysicalDevice physical_device, bool depth_only,
                                       const std::vector<VkFormat>& depth_format_priority_list)
    {
        VkFormat depth_format{VK_FORMAT_UNDEFINED};

        for (auto& format : depth_format_priority_list)
        {
            if (depth_only && !is_depth_only_format(format))
            {
                continue;
            }

            VkFormatProperties properties;
            vkGetPhysicalDeviceFormatProperties(physical_device, format, &properties);

            // Format must support depth stencil attachment for optimal tiling
            if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                depth_format = format;
                break;
            }
        }

        if (depth_format != VK_FORMAT_UNDEFINED)
        {
            LOG_INFO("Depth format selected: {}", to_string(depth_format));
            return depth_format;
        }

        throw std::runtime_error("No suitable depth format could be determined");
    }

    VkFormat choose_blendable_format(VkPhysicalDevice physical_device,
                                     const std::vector<VkFormat>& format_priority_list)
    {
        for (const auto& format : format_priority_list)
        {
            VkFormatProperties properties;
            vkGetPhysicalDeviceFormatProperties(physical_device, format, &properties);
            if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT)
            {
                return format;
            }
        }

        throw std::runtime_error("No suitable blendable format could be determined");
    }

    void make_filters_valid(VkPhysicalDevice physical_device, VkFormat format, VkFilter* filter,
                            VkSamplerMipmapMode* mipmapMode)
    {
        // Not all formats support linear filtering, so we need to adjust them if they don't
        if (*filter == VK_FILTER_NEAREST && (mipmapMode == nullptr || *mipmapMode == VK_SAMPLER_MIPMAP_MODE_NEAREST))
        {
            return; // These must already be valid
        }

        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(physical_device, format, &properties);

        if (!(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        {
            *filter = VK_FILTER_NEAREST;
            if (mipmapMode)
            {
                *mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            }
        }
    }

    bool is_dynamic_buffer_descriptor_type(VkDescriptorType descriptor_type)
    {
        return descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC ||
            descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    }

    bool is_buffer_descriptor_type(VkDescriptorType descriptor_type)
    {
        return descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
            descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
            is_dynamic_buffer_descriptor_type(descriptor_type);
    }

    int32_t get_bits_per_pixel(VkFormat format)
    {
        switch (format)
        {
        case VK_FORMAT_R4G4_UNORM_PACK8:
            return 8;
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_B5G6R5_UNORM_PACK16:
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
            return 16;
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_R8_SNORM:
        case VK_FORMAT_R8_USCALED:
        case VK_FORMAT_R8_SSCALED:
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_R8_SINT:
        case VK_FORMAT_R8_SRGB:
            return 8;
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_R8G8_SNORM:
        case VK_FORMAT_R8G8_USCALED:
        case VK_FORMAT_R8G8_SSCALED:
        case VK_FORMAT_R8G8_UINT:
        case VK_FORMAT_R8G8_SINT:
        case VK_FORMAT_R8G8_SRGB:
            return 16;
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8B8_SNORM:
        case VK_FORMAT_R8G8B8_USCALED:
        case VK_FORMAT_R8G8B8_SSCALED:
        case VK_FORMAT_R8G8B8_UINT:
        case VK_FORMAT_R8G8B8_SINT:
        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_B8G8R8_UNORM:
        case VK_FORMAT_B8G8R8_SNORM:
        case VK_FORMAT_B8G8R8_USCALED:
        case VK_FORMAT_B8G8R8_SSCALED:
        case VK_FORMAT_B8G8R8_UINT:
        case VK_FORMAT_B8G8R8_SINT:
        case VK_FORMAT_B8G8R8_SRGB:
            return 24;
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_R8G8B8A8_USCALED:
        case VK_FORMAT_R8G8B8A8_SSCALED:
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_R8G8B8A8_SINT:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_SNORM:
        case VK_FORMAT_B8G8R8A8_USCALED:
        case VK_FORMAT_B8G8R8A8_SSCALED:
        case VK_FORMAT_B8G8R8A8_UINT:
        case VK_FORMAT_B8G8R8A8_SINT:
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
            return 32;
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:
        case VK_FORMAT_A2R10G10B10_SINT_PACK32:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
        case VK_FORMAT_A2B10G10R10_SINT_PACK32:
            return 32;
        case VK_FORMAT_R16_UNORM:
        case VK_FORMAT_R16_SNORM:
        case VK_FORMAT_R16_USCALED:
        case VK_FORMAT_R16_SSCALED:
        case VK_FORMAT_R16_UINT:
        case VK_FORMAT_R16_SINT:
        case VK_FORMAT_R16_SFLOAT:
            return 16;
        case VK_FORMAT_R16G16_UNORM:
        case VK_FORMAT_R16G16_SNORM:
        case VK_FORMAT_R16G16_USCALED:
        case VK_FORMAT_R16G16_SSCALED:
        case VK_FORMAT_R16G16_UINT:
        case VK_FORMAT_R16G16_SINT:
        case VK_FORMAT_R16G16_SFLOAT:
            return 32;
        case VK_FORMAT_R16G16B16_UNORM:
        case VK_FORMAT_R16G16B16_SNORM:
        case VK_FORMAT_R16G16B16_USCALED:
        case VK_FORMAT_R16G16B16_SSCALED:
        case VK_FORMAT_R16G16B16_UINT:
        case VK_FORMAT_R16G16B16_SINT:
        case VK_FORMAT_R16G16B16_SFLOAT:
            return 48;
        case VK_FORMAT_R16G16B16A16_UNORM:
        case VK_FORMAT_R16G16B16A16_SNORM:
        case VK_FORMAT_R16G16B16A16_USCALED:
        case VK_FORMAT_R16G16B16A16_SSCALED:
        case VK_FORMAT_R16G16B16A16_UINT:
        case VK_FORMAT_R16G16B16A16_SINT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return 64;
        case VK_FORMAT_R32_UINT:
        case VK_FORMAT_R32_SINT:
        case VK_FORMAT_R32_SFLOAT:
            return 32;
        case VK_FORMAT_R32G32_UINT:
        case VK_FORMAT_R32G32_SINT:
        case VK_FORMAT_R32G32_SFLOAT:
            return 64;
        case VK_FORMAT_R32G32B32_UINT:
        case VK_FORMAT_R32G32B32_SINT:
        case VK_FORMAT_R32G32B32_SFLOAT:
            return 96;
        case VK_FORMAT_R32G32B32A32_UINT:
        case VK_FORMAT_R32G32B32A32_SINT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return 128;
        case VK_FORMAT_R64_UINT:
        case VK_FORMAT_R64_SINT:
        case VK_FORMAT_R64_SFLOAT:
            return 64;
        case VK_FORMAT_R64G64_UINT:
        case VK_FORMAT_R64G64_SINT:
        case VK_FORMAT_R64G64_SFLOAT:
            return 128;
        case VK_FORMAT_R64G64B64_UINT:
        case VK_FORMAT_R64G64B64_SINT:
        case VK_FORMAT_R64G64B64_SFLOAT:
            return 192;
        case VK_FORMAT_R64G64B64A64_UINT:
        case VK_FORMAT_R64G64B64A64_SINT:
        case VK_FORMAT_R64G64B64A64_SFLOAT:
            return 256;
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
            return 32;
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
            return 32;
        case VK_FORMAT_D16_UNORM:
            return 16;
        case VK_FORMAT_X8_D24_UNORM_PACK32:
            return 32;
        case VK_FORMAT_D32_SFLOAT:
            return 32;
        case VK_FORMAT_S8_UINT:
            return 8;
        case VK_FORMAT_D16_UNORM_S8_UINT:
            return 24;
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return 32;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return 40;
        case VK_FORMAT_UNDEFINED:
        default:
            return -1;
        }
    }

    std::vector<VkImageCompressionFixedRateFlagBitsEXT> fixed_rate_compression_flags_to_vector(
        VkImageCompressionFixedRateFlagsEXT flags)
    {
        const std::vector<VkImageCompressionFixedRateFlagBitsEXT> all_flags = {
            VK_IMAGE_COMPRESSION_FIXED_RATE_1BPC_BIT_EXT, VK_IMAGE_COMPRESSION_FIXED_RATE_2BPC_BIT_EXT,
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
            VK_IMAGE_COMPRESSION_FIXED_RATE_23BPC_BIT_EXT, VK_IMAGE_COMPRESSION_FIXED_RATE_24BPC_BIT_EXT
        };

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

    VkImageCompressionPropertiesEXT query_supported_fixed_rate_compression(
        VkPhysicalDevice gpu, const VkImageCreateInfo& create_info)
    {
        VkImageCompressionPropertiesEXT supported_compression_properties{
            VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_PROPERTIES_EXT
        };

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
