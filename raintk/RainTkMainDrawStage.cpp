/*
   Copyright (C) 2015 Preet Desai (preet.desai@gmail.com)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <algorithm>

#include <glm/gtx/string_cast.hpp>

#include <ks/gl/KsGLCommands.hpp>
#include <raintk/RainTkLog.hpp>
#include <raintk/RainTkMainDrawStage.hpp>


namespace raintk
{
    void MainDrawStage::SyncView(glm::vec4 const &viewport,
                                 ks::gl::Camera<float> const &camera)
    {
        m_viewport = viewport;
        m_camera = camera;
    }

    void MainDrawStage::SyncClipRegions(std::vector<BoundingBox> const &list_clip_regions)
    {
        m_list_clip_regions = list_clip_regions;
    }

    void MainDrawStage::SyncDrawOrder(std::vector<Id> const &list_opq_draw_order,
                                      std::vector<Id> const &list_xpr_draw_order)
    {
        m_list_opq_draw_order = list_opq_draw_order;
        m_list_xpr_draw_order = list_xpr_draw_order;
    }

    void MainDrawStage::Render(ks::draw::DrawParams<DrawKey> &p)
    {
        auto const &camera = m_camera;
        auto const &viewport = m_viewport;

        // reset stats
        this->m_stats.reset();

        // get params
        auto& list_draw_calls = p.list_draw_calls;

        // Setup viewport (all values in pixels)
        // z == width, w == height
        ks::gl::Viewport(viewport.x,viewport.y,viewport.z,viewport.w);

        // Enable scissor test to allow for clipping
        p.state_set->SetScissorTest(GL_TRUE);

        // Enable depth writes to clear the buffer
        p.state_set->SetDepthMask(GL_TRUE);
        p.state_set->SetClearColor(0.15,0.15,0.15,1.0);

        // Set the scissor extents to the viewport so that
        // everything cleared
        ks::gl::Scissor(viewport.x,viewport.y,viewport.z,viewport.w);

        ks::gl::Clear(ks::gl::ColorBufferBit |
                      ks::gl::DepthBufferBit |
                      ks::gl::StencilBufferBit);

        DrawKey prev_key; // key value should be 0

        // Opaque draw calls
        for(auto const dc_id : m_list_opq_draw_order)
        {
            auto& draw_call = list_draw_calls[dc_id];
            setupState(viewport,camera,p,prev_key,draw_call.key);

            ks::gl::ShaderProgram* shader =
                    p.list_shaders[draw_call.key.GetShader()].get();

            // Set the individual uniforms
            if(draw_call.list_uniforms)
            {
                auto &list_uniforms = *(draw_call.list_uniforms);
                for(auto &uniform : list_uniforms) {
                    uniform->GLSetUniform(shader);
                }
            }

            // Draw!
            issueDrawCall(shader,draw_call);
        }

        // Transparent draw calls
        for(auto const dc_id : m_list_xpr_draw_order)
        {
            auto& draw_call = list_draw_calls[dc_id];
            setupState(viewport,camera,p,prev_key,draw_call.key);

            ks::gl::ShaderProgram* shader =
                    p.list_shaders[draw_call.key.GetShader()].get();

            // Set the individual uniforms
            if(draw_call.list_uniforms)
            {
                auto &list_uniforms = *(draw_call.list_uniforms);
                for(auto &uniform : list_uniforms) {
                    uniform->GLSetUniform(shader);
                }
            }

            // Draw!
            issueDrawCall(shader,draw_call);
        }

        // Disable scissor test to reset to
        // defaults for other draw stages
        p.state_set->SetScissorTest(GL_FALSE);
    }

    void MainDrawStage::Reset()
    {
        // TODO
    }

    void MainDrawStage::setupState(glm::vec4 const &viewport,
                                   ks::gl::Camera<float> const &camera,
                                   ks::draw::DrawParams<DrawKey>& p,
                                   DrawKey& prev_key,
                                   DrawKey const curr_key)
    {
        if(!(prev_key == curr_key))
        {
            auto const clip_id = curr_key.GetClip();
            auto const shader_id = curr_key.GetShader();
            auto const depth_config_id = curr_key.GetDepthConfig();
            auto const blend_config_id = curr_key.GetBlendConfig();
            auto const stencil_config_id = curr_key.GetStencilConfig();
            auto const texture_set_id = curr_key.GetTextureSet();
            auto const uniform_set_id = curr_key.GetUniformSet();

            if(prev_key.GetClip() != curr_key.GetClip())
            {
                glm::mat4 const m4_pv =
                        camera.GetProjMatrix()*
                        camera.GetViewMatrix();

                // Clip structure:
                // NOTE: The 'z' and 'w' are NOT width and height
                // glm vec4 [x,y,z,w]: [x0,y0,x1,y1]
                auto const &clip_bbox = m_list_clip_regions[clip_id];
                glm::vec4 const clip(
                            clip_bbox.x0,
                            clip_bbox.y0,
                            clip_bbox.x1,
                            clip_bbox.y1);

                auto const ndc_clip_bl = m4_pv*glm::vec4(clip.x,clip.w,0,1);
                auto const ndc_clip_tr = m4_pv*glm::vec4(clip.z,clip.y,0,1);

                float clip_x = 0.5*(ndc_clip_bl.x+1.0)*viewport.z;
                float clip_y = 0.5*(ndc_clip_bl.y+1.0)*viewport.w;
                float clip_w = (0.5*(ndc_clip_tr.x+1.0))*viewport.z - clip_x;
                float clip_h = (0.5*(ndc_clip_tr.y+1.0))*viewport.w - clip_y;

                // TODO clip should consider viewport x,y; currently we
                // only consider the width and height of the viewport
                ks::gl::Scissor(clip_x,clip_y,clip_w,clip_h);
            }

            if(prev_key.GetShader() != curr_key.GetShader())
            {
                auto& shader = p.list_shaders[shader_id];

                glm::mat4 const u_m4_pv =
                        camera.GetProjMatrix()*
                        camera.GetViewMatrix();

                shader->GLEnable(p.state_set);
                shader->GLSetUniform("u_m4_pv",u_m4_pv);
                this->m_stats.shader_switches++;
            }

            if((prev_key.GetDepthConfig() != depth_config_id) && (depth_config_id > 0))
            {
                p.list_depth_configs[depth_config_id](p.state_set);
                this->m_stats.raster_ops++;
            }

            if((prev_key.GetBlendConfig() != blend_config_id) && (blend_config_id > 0))
            {
                p.list_blend_configs[blend_config_id](p.state_set);
                this->m_stats.raster_ops++;
            }

            if((prev_key.GetStencilConfig() != stencil_config_id) && (stencil_config_id > 0))
            {
                p.list_stencil_configs[stencil_config_id](p.state_set);
                this->m_stats.raster_ops++;
            }

            if(prev_key.GetTextureSet() != texture_set_id)
            {
                auto& texture_set = p.list_texture_sets[texture_set_id];

                for(auto& desc : texture_set->list_texture_desc)
                {
                    auto& texture = desc.first;
                    auto tex_unit = desc.second;

                    texture->GLBind(p.state_set,tex_unit);
                    this->m_stats.texture_switches++;
                }
            }

            if(prev_key.GetUniformSet() != uniform_set_id)
            {
                auto& uniform_set = p.list_uniform_sets[uniform_set_id];
                auto& shader = p.list_shaders[shader_id];

                for(auto& uniform : uniform_set->list_uniforms)
                {
                    uniform->GLSetUniform(shader.get());
                }
            }

            prev_key = curr_key;
        }
    }

    void MainDrawStage::issueDrawCall(ks::gl::ShaderProgram* shader,
                                      DrawCall &draw_call)
    {
        // Draw
        // TODO: Since we are drawing all buffers in
        // a tight loop, we might be able to check
        // when we really need to bind/unbind buffers
        // to avoid redundant calls

        auto primitive = draw_call.key.GetPrimitive();

        if(draw_call.draw_ix.buffer) {
            // bind vertex buffers
            bool ok = true;
            for(auto& range : draw_call.list_draw_vx) {
                ok = ok && range.buffer->GLBindVxBuff(
                            shader,range.start_byte);
            }

            // bind index buffer
            ok = ok && draw_call.draw_ix.buffer->GLBind();

            assert(ok);

            ks::gl::DrawElements(
                        primitive,
                        draw_call.draw_ix.start_byte,
                        draw_call.draw_ix.size_bytes);

            for(auto& range : draw_call.list_draw_vx) {
                range.buffer->GLUnbind();
            }
            draw_call.draw_ix.buffer->GLUnbind();
        }
        else {
            // bind vertex buffers
            bool ok = true;
            for(auto& range : draw_call.list_draw_vx) {
                ok = ok && range.buffer->GLBindVxBuff(
                            shader,range.start_byte);
            }

            auto& first_range = draw_call.list_draw_vx[0];

            ks::gl::DrawArrays(
                        primitive,
                        first_range.buffer->GetVertexSizeBytes(),
                        0,first_range.size_bytes);

            for(auto& range : draw_call.list_draw_vx) {
                range.buffer->GLUnbind();
            }
        }

        this->m_stats.draw_calls++;
    }
}
