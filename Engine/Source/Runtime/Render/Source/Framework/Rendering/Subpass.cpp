#include "Framework/Rendering/Subpass.hpp"
#include "Framework/Rendering/RenderTarget.hpp"

namespace vkb
{

    inline Subpass::Subpass(vkb::RenderContext &render_context, ShaderSource &&vertex_source, ShaderSource &&fragment_source)
        : render_context{render_context},
          vertex_shader{std::move(vertex_source)},
          fragment_shader{std::move(fragment_source)}
    {
    }

    inline const std::vector<uint32_t> &Subpass::get_input_attachments() const
    {
        return input_attachments;
    }

    inline LightingState &Subpass::get_lighting_state()
    {
        return lighting_state;
    }

    inline const std::vector<uint32_t> &Subpass::get_output_attachments() const
    {
        return output_attachments;
    }

    inline typename vkb::RenderContext &Subpass::get_render_context()
    {
        return render_context;
    }

    std::unordered_map<std::string, ShaderResourceMode> const &Subpass::get_resource_mode_map() const
    {
        return resource_mode_map;
    }

    inline typename VkSampleCountFlagBits Subpass::get_sample_count() const
    {
        return sample_count;
    }

    inline const ShaderSource &Subpass::get_vertex_shader() const
    {
        return vertex_shader;
    }

    inline const std::vector<uint32_t> &Subpass::get_color_resolve_attachments() const
    {
        return color_resolve_attachments;
    }

    inline const std::string &Subpass::get_debug_name() const
    {
        return debug_name;
    }

    inline const uint32_t &Subpass::get_depth_stencil_resolve_attachment() const
    {
        return depth_stencil_resolve_attachment;
    }

    inline typename VkResolveModeFlagBits Subpass::get_depth_stencil_resolve_mode() const
    {
        return depth_stencil_resolve_mode;
    }

    inline typename vkb::DepthStencilState &Subpass::get_depth_stencil_state()
    {
        return depth_stencil_state;
    }

    inline const bool &Subpass::get_disable_depth_stencil_attachment() const
    {
        return disable_depth_stencil_attachment;
    }

    inline const ShaderSource &Subpass::get_fragment_shader() const
    {
        return fragment_shader;
    }

    inline void Subpass::set_color_resolve_attachments(std::vector<uint32_t> const &color_resolve)
    {
        color_resolve_attachments = color_resolve;
    }

    inline void Subpass::set_depth_stencil_resolve_attachment(uint32_t depth_stencil_resolve)
    {
        depth_stencil_resolve_attachment = depth_stencil_resolve;
    }

    inline void Subpass::set_debug_name(const std::string &name)
    {
        debug_name = name;
    }

    inline void Subpass::set_disable_depth_stencil_attachment(bool disable_depth_stencil)
    {
        disable_depth_stencil_attachment = disable_depth_stencil;
    }

    inline void Subpass::set_depth_stencil_resolve_mode(VkResolveModeFlagBits mode)
    {
        depth_stencil_resolve_mode = mode;
    }

    inline void Subpass::set_input_attachments(std::vector<uint32_t> const &input)
    {
        input_attachments = input;
    }

    inline void Subpass::set_output_attachments(std::vector<uint32_t> const &output)
    {
        output_attachments = output;
    }

    inline void Subpass::set_sample_count(VkSampleCountFlagBits sample_count_)
    {
        sample_count = sample_count_;
    }

    inline void Subpass::update_render_target_attachments(RenderTarget &render_target)
    {
        render_target.set_input_attachments(input_attachments);
        render_target.set_output_attachments(output_attachments);
    }
}