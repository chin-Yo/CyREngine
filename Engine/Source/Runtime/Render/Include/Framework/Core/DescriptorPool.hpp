#pragma once
#include <vulkan/vulkan.hpp>

namespace vks
{

    class DescriptorPool
    {
    public:
        DescriptorPool(VkDevice device, VkDescriptorPool pool);
        ~DescriptorPool();

        // 禁止拷贝，但允许移动
        DescriptorPool(const DescriptorPool &) = delete;
        DescriptorPool &operator=(const DescriptorPool &) = delete;
        DescriptorPool(DescriptorPool &&other) noexcept;
        DescriptorPool &operator=(DescriptorPool &&other) noexcept;

        // 分配描述符集
        VkDescriptorSet allocateSet(VkDescriptorSetLayout layout);

        // 重置池
        void reset();

        VkDescriptorPool get() const { return m_pool; }

    private:
        VkDevice m_device = VK_NULL_HANDLE;
        VkDescriptorPool m_pool = VK_NULL_HANDLE;
    };

    class DescriptorPoolBuilder
    {
    public:
        DescriptorPoolBuilder(VkDevice device);

        DescriptorPoolBuilder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
        DescriptorPoolBuilder &setMaxSets(uint32_t count);
        DescriptorPoolBuilder &setPoolFlags(VkDescriptorPoolCreateFlags flags);

        DescriptorPool build();

    private:
        VkDevice m_device;
        std::vector<VkDescriptorPoolSize> m_poolSizes;
        uint32_t m_maxSets = 1000;
        VkDescriptorPoolCreateFlags m_poolFlags = 0;
    };

}