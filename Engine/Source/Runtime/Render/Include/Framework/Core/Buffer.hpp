#pragma once

#include "Framework/Core/Allocated.hpp"
#include "Framework/Misc/BuilderBase.hpp"
#include <memory>
#include <vector>
class VulkanDevice;

namespace vkb
{
    class Buffer;


    class BufferBuilder
        : public BuilderBase<BufferBuilder, VkBufferCreateInfo>
    {
    public:
        // C API 类型别名
        using BufferCreateFlagsType = VkBufferCreateFlags;
        using BufferCreateInfoType = VkBufferCreateInfo;
        using BufferUsageFlagsType = VkBufferUsageFlags;
        using DeviceSizeType = VkDeviceSize;
        using SharingModeType = VkSharingMode;
        using DeviceType = VulkanDevice;

    private:
        using ParentType = BuilderBase<BufferBuilder, VkBufferCreateInfo>;

    public:
        // 构造函数
        BufferBuilder(DeviceSizeType size)
            : // 初始化基类
            ParentType(VkBufferCreateInfo{
                VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, // sType
                nullptr, // pNext
                0, // flags
                size, // size
                0, // usage (稍后通过 with_usage 设置)
                VK_SHARING_MODE_EXCLUSIVE, // sharingMode
                0, // queueFamilyIndexCount
                nullptr // pQueueFamilyIndices
            })
        {
        }

        // 构建方法 (声明在此，定义在 Buffer 类完整定义之后)
        Buffer build(DeviceType& device) const;
        std::unique_ptr<Buffer> build_unique(DeviceType& device) const;

        // 链式调用方法
        BufferBuilder& with_flags(BufferCreateFlagsType flags)
        {
            this->get_create_info().flags = flags;
            return *this;
        }

        BufferBuilder& with_usage(BufferUsageFlagsType usage)
        {
            this->get_create_info().usage = usage;
            return *this;
        }
    };

    class Buffer : public Allocated<VkBuffer>
    {
    public:
        // 类型别名
        using BufferType = VkBuffer;
        using BufferUsageFlagsType = VkBufferUsageFlags;
        using DeviceSizeType = VkDeviceSize;
        using DeviceType = VulkanDevice;

    private:
        using ParentType = Allocated<VkBuffer>;

    public:
        // 静态工厂函数
        static Buffer create_staging_buffer(DeviceType& device, DeviceSizeType size, const void* data);

        template <typename T>
        static Buffer create_staging_buffer(DeviceType& device, const std::vector<T>& data);

        template <typename T>
        static Buffer create_staging_buffer(DeviceType& device, const T& data);

        // 构造函数与析构函数 (声明)
        Buffer() = delete;
        Buffer(const Buffer&) = delete;
        Buffer(Buffer&& other) = default;
        Buffer& operator=(const Buffer&) = delete;
        Buffer& operator=(Buffer&&) = default;
        ~Buffer() override;

        Buffer(DeviceType& device,
               DeviceSizeType size,
               BufferUsageFlagsType buffer_usage,
               VmaMemoryUsage memory_usage,
               VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
                   VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
               const std::vector<uint32_t>& queue_family_indices = {});

        Buffer(DeviceType& device, const BufferBuilder& builder);

        // 成员函数
        uint64_t get_device_address() const;

        DeviceSizeType get_size() const { return size; }

    private:
        DeviceSizeType size = 0;
    };


    inline Buffer BufferBuilder::build(DeviceType& device) const
    {
        return Buffer{device, *this};
    }

    inline std::unique_ptr<Buffer> BufferBuilder::build_unique(DeviceType& device) const
    {
        return std::make_unique<Buffer>(device, *this);
    }

    template <typename T>
    Buffer Buffer::create_staging_buffer(DeviceType& device, const T& data)
    {
        return create_staging_buffer(device, sizeof(T), &data);
    }

    template <typename T>
    Buffer Buffer::create_staging_buffer(DeviceType& device, const std::vector<T>& data)
    {
        return create_staging_buffer(device, data.size() * sizeof(T), data.data());
    }
} // namespace vkb
