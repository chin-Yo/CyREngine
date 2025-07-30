#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>

namespace vks
{
    class DescriptorSetLayout
    {
    public:
        DescriptorSetLayout(VkDevice device, VkDescriptorSetLayout layout);
        ~DescriptorSetLayout();

        // 禁止拷贝，允许移动
        DescriptorSetLayout(const DescriptorSetLayout &) = delete;
        DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;
        DescriptorSetLayout(DescriptorSetLayout &&other) noexcept;
        DescriptorSetLayout &operator=(DescriptorSetLayout &&other) noexcept;

        VkDescriptorSetLayout get() const { return m_layout; }
        operator VkDescriptorSetLayout() const { return m_layout; }

    private:
        VkDevice m_device;
        VkDescriptorSetLayout m_layout;
    };

    class DescriptorSetLayoutBuilder
    {
    public:
        DescriptorSetLayoutBuilder(VkDevice device);

        // 添加一个绑定
        DescriptorSetLayoutBuilder &addBinding(
            uint32_t binding,
            VkDescriptorType descriptorType,
            VkShaderStageFlags stageFlags,
            uint32_t descriptorCount = 1);

        DescriptorSetLayout build();

    private:
        VkDevice m_device;
        std::vector<VkDescriptorSetLayoutBinding> m_bindings;
    };
}