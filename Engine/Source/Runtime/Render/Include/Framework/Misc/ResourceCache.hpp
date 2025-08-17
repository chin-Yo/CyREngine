#pragma once

#include <mutex>

#include "Framework/Core/PipelineLayout.hpp"
#include "Framework/Core/DescriptorSetLayout.hpp"
#include "Framework/Core/DescriptorPool.hpp"
#include "Framework/Core/RenderPass.hpp"
#include "Framework/Core/Pipeline.hpp"
#include "Framework/Core/DescriptorSet.hpp"
#include "Framework/Core/Framebuffer.hpp"


namespace vkb
{
    class VulkanDevice;
    /**
     * @brief Struct to hold the internal state of the Resource Cache
     *
     */
    struct ResourceCacheState
    {
        std::unordered_map<std::size_t, ShaderModule> shader_modules;

        std::unordered_map<std::size_t, PipelineLayout> pipeline_layouts;

        std::unordered_map<std::size_t, DescriptorSetLayout> descriptor_set_layouts;

        std::unordered_map<std::size_t, DescriptorPool> descriptor_pools;

        std::unordered_map<std::size_t, RenderPass> render_passes;

        std::unordered_map<std::size_t, GraphicsPipeline> graphics_pipelines;

        std::unordered_map<std::size_t, ComputePipeline> compute_pipelines;

        std::unordered_map<std::size_t, DescriptorSet> descriptor_sets;

        std::unordered_map<std::size_t, Framebuffer> framebuffers;
    };

    class ResourceCache
    {
    public:
        ResourceCache(VulkanDevice &device);

        ResourceCache(const ResourceCache &) = delete;

        ResourceCache(ResourceCache &&) = delete;

        ResourceCache &operator=(const ResourceCache &) = delete;

        ResourceCache &operator=(ResourceCache &&) = delete;

    private:
        VulkanDevice &device;

        VkPipelineCache pipeline_cache{VK_NULL_HANDLE};

        ResourceCacheState state;

        std::mutex descriptor_set_mutex;

        std::mutex pipeline_layout_mutex;

        std::mutex shader_module_mutex;

        std::mutex descriptor_set_layout_mutex;

        std::mutex graphics_pipeline_mutex;

        std::mutex render_pass_mutex;

        std::mutex compute_pipeline_mutex;

        std::mutex framebuffer_mutex;
    };
}