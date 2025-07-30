#include "Framework/Core/DescriptorSetLayout.hpp"

namespace vks
{
    DescriptorSetLayout::DescriptorSetLayout(VkDevice device, VkDescriptorSetLayout layout)
        : m_device(device), m_layout(layout) {}

    DescriptorSetLayout::~DescriptorSetLayout()
    {
        if (m_layout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
            m_layout = VK_NULL_HANDLE;
        }
    }

    DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout &&other) noexcept
        : m_device(other.m_device), m_layout(other.m_layout)
    {
        // 窃取资源，并将源对象置于无效状态
        other.m_device = VK_NULL_HANDLE;
        other.m_layout = VK_NULL_HANDLE;
    }

    DescriptorSetLayout &DescriptorSetLayout::operator=(DescriptorSetLayout &&other) noexcept
    {
        if (this != &other)
        {
            // 释放当前持有的资源
            if (m_layout != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
            }

            // 窃取新资源
            m_device = other.m_device;
            m_layout = other.m_layout;

            // 将源对象置于无效状态
            other.m_device = VK_NULL_HANDLE;
            other.m_layout = VK_NULL_HANDLE;
        }
        return *this;
    }

    DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder(VkDevice device)
        : m_device(device) {}

    DescriptorSetLayoutBuilder &DescriptorSetLayoutBuilder::addBinding(
        uint32_t binding, VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags, uint32_t descriptorCount)
    {

        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = descriptorCount;
        layoutBinding.stageFlags = stageFlags;
        layoutBinding.pImmutableSamplers = nullptr;

        m_bindings.push_back(layoutBinding);
        return *this;
    }

    DescriptorSetLayout DescriptorSetLayoutBuilder::build()
    {
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(m_bindings.size());
        layoutInfo.pBindings = m_bindings.data();

        VkDescriptorSetLayout layout;
        if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &layout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create descriptor set layout!");
        }

        // 创建 RAII 对象并返回
        return DescriptorSetLayout(m_device, layout);
    }
}