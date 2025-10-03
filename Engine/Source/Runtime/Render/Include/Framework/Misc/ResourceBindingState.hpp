#pragma once

#include "Framework/Common/VkCommon.hpp"

namespace vkb
{
    class Buffer;
    class ImageView;
    class Sampler;
    
    /**
     * @brief 包含实际资源数据的结构体。
     *
     * 此结构体将被描述符集内的 buffer info 或 image info 引用。
     */
    struct ResourceInfo
    {
        bool dirty{false};

        const vkb::Buffer *buffer{nullptr};

        VkDeviceSize offset{0};

        VkDeviceSize range{0};

        const ImageView *image_view{nullptr};

        const Sampler *sampler{nullptr};
    };

    /**
     * @brief 资源集是一组包含由命令缓冲区绑定的资源的绑定。
     *
     * ResourceSet 与 DescriptorSet 具有一一对应的关系。
     */
    class ResourceSet
    {
    public:
        void reset();

        bool is_dirty() const;

        void clear_dirty();

        void clear_dirty(uint32_t binding, uint32_t array_element);

        // 规则 1 & 4: vkb::core::BufferC -> vkb::Buffer
        void bind_buffer(const vkb::Buffer &buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t binding, uint32_t array_element);

        // 规则 1: core::ImageView & core::Sampler -> vkb::ImageView & vkb::Sampler
        void bind_image(const vkb::ImageView &image_view, const vkb::Sampler &sampler, uint32_t binding, uint32_t array_element);

        void bind_image(const vkb::ImageView &image_view, uint32_t binding, uint32_t array_element);

        void bind_input(const vkb::ImageView &image_view, uint32_t binding, uint32_t array_element);

        // 规则 2: BindingMap 的模板参数已是 ResourceInfo，本身不含 Cpp/C API 类型，无需更改
        const BindingMap<ResourceInfo> &get_resource_bindings() const;

    private:
        bool dirty{false};

        BindingMap<ResourceInfo> resource_bindings;
    };

    /**
     * @brief 命令缓冲区的资源绑定状态。
     *
     * 跟踪命令缓冲区绑定的所有资源。ResourceBindingState 被命令缓冲区
     * 用于在绘制时创建适当的描述符集。
     */
    class ResourceBindingState
    {
    public:
        void reset();

        bool is_dirty();

        void clear_dirty();

        void clear_dirty(uint32_t set);

        // 规则 1 & 4: vkb::core::BufferC -> vkb::Buffer
        void bind_buffer(const vkb::Buffer &buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t set, uint32_t binding, uint32_t array_element);

        // 规则 1: core::ImageView & core::Sampler -> vkb::ImageView & vkb::Sampler
        void bind_image(const vkb::ImageView &image_view, const vkb::Sampler &sampler, uint32_t set, uint32_t binding, uint32_t array_element);

        void bind_image(const vkb::ImageView &image_view, uint32_t set, uint32_t binding, uint32_t array_element);

        void bind_input(const vkb::ImageView &image_view, uint32_t set, uint32_t binding, uint32_t array_element);

        const std::unordered_map<uint32_t, ResourceSet> &get_resource_sets();

    private:
        bool dirty{false};

        std::unordered_map<uint32_t, ResourceSet> resource_sets;
    };
} // namespace vkb