#pragma once

#include "Framework/Common/glmCommon.hpp"
#include <vector>
#include "Framework/Misc/BufferPool.hpp"
#include "Framework/Core/ShaderModule.hpp"
#include "Framework/Core/PipelineState.hpp"

namespace vkb
{
    class RenderContext;
    class CommandBuffer;
    class RenderTarget;

    struct alignas(16) Light
    {
        glm::vec4 position;  // position.w represents type of light
        glm::vec4 color;     // color.w represents light intensity
        glm::vec4 direction; // direction.w represents range
        glm::vec2 info;      // (only used for spot lights) info.x represents light inner cone angle, info.y represents light outer cone angle
    };

    struct LightingState
    {
        std::vector<Light> directional_lights;
        std::vector<Light> point_lights;
        std::vector<Light> spot_lights;
        BufferAllocation light_buffer;
    };

    class Subpass
    {
    public:
        // 构造函数，接收渲染上下文和着色器源码
        Subpass(vkb::RenderContext &render_context, ShaderSource &&vertex_shader, ShaderSource &&fragment_shader);

        // 禁用拷贝构造和拷贝赋值，防止意外的昂贵拷贝操作
        Subpass(const Subpass &) = delete;
        Subpass &operator=(const Subpass &) = delete;

        // 使用默认的移动构造和析构函数
        Subpass(Subpass &&) = default;
        virtual ~Subpass() = default;

        // 禁用移动赋值操作
        Subpass &operator=(Subpass &&) = delete;

        /**
         * @brief 纯虚函数，用于将绘制命令记录到指定的命令缓冲区中。
         * @param command_buffer 用于记录绘制命令的命令缓冲区。
         */
        virtual void draw(vkb::CommandBuffer &command_buffer) = 0;

        /**
         * @brief 纯虚函数，用于准备子通道所需的着色器和着色器变体。
         */
        virtual void prepare() = 0;

        /**
         * @brief 分配和准备场景光照数据。
         *
         * @tparam T 一个包含 'directional_lights', 'point_lights', 'spot_light' 成员的灯光结构体。
         * @param scene_lights 场景图中所有的灯光组件。
         * @param max_lights_per_type 每种类型光照所允许的最大数量。
         */
        template <typename T>
        void allocate_lights(const std::vector<class Light *> &scene_lights,
                             size_t max_lights_per_type);

        // Getters
        const std::vector<uint32_t> &get_color_resolve_attachments() const;
        const std::string &get_debug_name() const;
        const uint32_t &get_depth_stencil_resolve_attachment() const;
        VkResolveModeFlagBits get_depth_stencil_resolve_mode() const;
        vkb::DepthStencilState &get_depth_stencil_state();
        const bool &get_disable_depth_stencil_attachment() const;
        const ShaderSource &get_fragment_shader() const;
        const std::vector<uint32_t> &get_input_attachments() const;
        vkb::LightingState &get_lighting_state();
        const std::vector<uint32_t> &get_output_attachments() const;
        vkb::RenderContext &get_render_context();
        const std::unordered_map<std::string, ShaderResourceMode> &get_resource_mode_map() const;
        VkSampleCountFlagBits get_sample_count() const;
        const ShaderSource &get_vertex_shader() const;

        // Setters
        void set_color_resolve_attachments(std::vector<uint32_t> const &color_resolve);
        void set_debug_name(const std::string &name);
        void set_disable_depth_stencil_attachment(bool disable_depth_stencil);
        void set_depth_stencil_resolve_attachment(uint32_t depth_stencil_resolve);
        void set_depth_stencil_resolve_mode(VkResolveModeFlagBits mode);
        void set_input_attachments(std::vector<uint32_t> const &input);
        void set_output_attachments(std::vector<uint32_t> const &output);
        void set_sample_count(VkSampleCountFlagBits sample_count);

        /**
         * @brief 使用此子通道中存储的附件更新渲染目标。
         *        此函数由RenderPipeline在开始渲染通道和进入新子通道之前调用。
         * @param render_target 需要更新的渲染目标。
         */
        void update_render_target_attachments(vkb::RenderTarget &render_target);

    private:
        // 颜色解析附件的索引列表，默认为空
        std::vector<uint32_t> color_resolve_attachments = {};

        // 用于调试的名称
        std::string debug_name{};

        /**
         * @brief 多重采样深度附件的解析模式。
         *        如果不是 VK_RESOLVE_MODE_NONE，将启用深度/模板附件的解析。
         */
        VkResolveModeFlagBits depth_stencil_resolve_mode{VK_RESOLVE_MODE_NONE};

        // 深度和模板测试状态
        vkb::DepthStencilState depth_stencil_state{};

        /**
         * @brief 如果为 true，则在创建渲染通道时禁用深度/模板附件。
         */
        bool disable_depth_stencil_attachment{false};

        // 深度/模板解析附件的索引，默认为不使用
        uint32_t depth_stencil_resolve_attachment{VK_ATTACHMENT_UNUSED};

        // 包含此子通道所需的所有光照数据的结构
        vkb::LightingState lighting_state{};

        // 输入附件的索引列表，默认为空
        std::vector<uint32_t> input_attachments = {};

        // 输出附件的索引列表，默认为附件0（通常是交换链图像）
        std::vector<uint32_t> output_attachments = {0};

        // 对渲染上下文的引用，该上下文拥有此子通道
        vkb::RenderContext &render_context;

        // 着色器资源名称到其资源模式的映射
        std::unordered_map<std::string, ShaderResourceMode> resource_mode_map;

        // 多重采样的样本计数
        VkSampleCountFlagBits sample_count{VK_SAMPLE_COUNT_1_BIT};

        // 顶点着色器源码
        ShaderSource vertex_shader;

        // 片段着色器源码
        ShaderSource fragment_shader;
    };
}