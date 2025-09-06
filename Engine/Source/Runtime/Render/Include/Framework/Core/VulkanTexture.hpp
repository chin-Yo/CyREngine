/*
 * Vulkan texture loader
 *
 * Copyright(C) by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license(MIT) (http://opensource.org/licenses/MIT)
 */

#pragma once
#include <stb_image.h>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <vector>

#include <volk.h>

#include <ktx.h>
#include <ktxvulkan.h>

#include "VulkanBuffer.hpp"
#include "VulkanTools.hpp"

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#endif

namespace vkb
{
    class VulkanDevice;
}

namespace vks
{
    class Texture
    {
    public:
        vkb::VulkanDevice *device;
        VkImage image;
        VkImageLayout imageLayout;
        VkDeviceMemory deviceMemory;
        VkImageView view;
        uint32_t width, height;
        uint32_t mipLevels;
        uint32_t layerCount;
        VkDescriptorImageInfo descriptor;
        VkSampler sampler;

        void updateDescriptor();
        void destroy();
        ktxResult loadKTXFile(std::string filename, ktxTexture **target);
    };

    class Texture2D : public Texture
    {
    public:
        void loadFromFile(
            std::string filename,
            VkFormat format,
            vkb::VulkanDevice *device,
            VkQueue copyQueue,
            VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
            VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            bool forceLinear = false);
        void loadFromPng(
            std::string filename,
            VkFormat format,
            vkb::VulkanDevice *device,
            VkQueue copyQueue,
            VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
            VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        void fromBuffer(
            void *buffer,
            VkDeviceSize bufferSize,
            VkFormat format,
            uint32_t texWidth,
            uint32_t texHeight,
            vkb::VulkanDevice *device,
            VkQueue copyQueue,
            VkFilter filter = VK_FILTER_LINEAR,
            VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
            VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    };

    class Texture2DArray : public Texture
    {
    public:
        void loadFromFile(
            std::string filename,
            VkFormat format,
            vkb::VulkanDevice *device,
            VkQueue copyQueue,
            VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
            VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    };

    class TextureCubeMap : public Texture
    {
    public:
        void loadFromFile(
            std::string filename,
            VkFormat format,
            vkb::VulkanDevice *device,
            VkQueue copyQueue,
            VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
            VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    };
} // namespace vks
