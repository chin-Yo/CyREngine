#include "Framework/Core/DescriptorPool.hpp"

namespace vks
{
    DescriptorPool::DescriptorPool(VkDevice device, VkDescriptorPool pool)
        : m_device(device), m_pool(pool) {}

    DescriptorPool::~DescriptorPool()
    {
        if (m_pool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(m_device, m_pool, nullptr);
            m_pool = VK_NULL_HANDLE;
        }
    }

    DescriptorPool::DescriptorPool(DescriptorPool &&other) noexcept
        : m_device(other.m_device), m_pool(other.m_pool)
    {
        other.m_device = VK_NULL_HANDLE;
        other.m_pool = VK_NULL_HANDLE;
    }

    DescriptorPool &DescriptorPool::operator=(DescriptorPool &&other) noexcept
    {
        if (this != &other)
        {
            if (m_pool != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorPool(m_device, m_pool, nullptr);
            }
            m_device = other.m_device;
            m_pool = other.m_pool;
            other.m_device = VK_NULL_HANDLE;
            other.m_pool = VK_NULL_HANDLE;
        }
        return *this;
    }

    VkDescriptorSet DescriptorPool::allocateSet(VkDescriptorSetLayout layout)
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;

        VkDescriptorSet descriptorSet;
        if (vkAllocateDescriptorSets(m_device, &allocInfo, &descriptorSet) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate descriptor set!");
        }
        return descriptorSet;
    }

    void DescriptorPool::reset()
    {
        vkResetDescriptorPool(m_device, m_pool, 0);
    }

    DescriptorPoolBuilder::DescriptorPoolBuilder(VkDevice device)
        : m_device(device) {}

    DescriptorPoolBuilder &DescriptorPoolBuilder::addPoolSize(VkDescriptorType descriptorType, uint32_t count)
    {
        m_poolSizes.push_back({descriptorType, count});
        return *this;
    }

    DescriptorPoolBuilder &DescriptorPoolBuilder::setMaxSets(uint32_t count)
    {
        m_maxSets = count;
        return *this;
    }

    DescriptorPoolBuilder &DescriptorPoolBuilder::setPoolFlags(VkDescriptorPoolCreateFlags flags)
    {
        m_poolFlags = flags;
        return *this;
    }

    DescriptorPool DescriptorPoolBuilder::build()
    {
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = m_poolFlags;
        poolInfo.maxSets = m_maxSets;
        poolInfo.poolSizeCount = static_cast<uint32_t>(m_poolSizes.size());
        poolInfo.pPoolSizes = m_poolSizes.data();

        VkDescriptorPool pool;
        if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &pool) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create descriptor pool!");
        }

        return DescriptorPool(m_device, pool);
    }
}