/*
   Copyright (C) 2016 Preet Desai (preet.desai@gmail.com)

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

#include <raintk/RainTkAtlasImage.hpp>
#include <raintk/RainTkDrawSystem.hpp>
#include <raintk/RainTkImageAtlas.hpp>
#include <raintk/RainTkScene.hpp>

namespace raintk
{
    // ============================================================= //
    // ============================================================= //

    namespace
    {
        #include <raintk/shaders/RainTkAtlasImage.glsl.hpp>

        struct Vertex
        {
            glm::vec4 a_v4_position; // 16
            glm::vec3 a_v3_tex0_opacity; // 8
        }; // sizeof == 28

        ks::gl::VertexLayout const g_vx_layout {
            { "a_v4_position", AttrType::Float, 4, false },
            { "a_v3_tex0_opacity", AttrType::Float, 3, false }
        };

        shared_ptr<GeometryLayout> g_geometry_layout(
            new GeometryLayout{
            g_vx_layout,
            sizeof(Vertex),
            24*6*128 // buffer size in bytes
        });

        // Setup
        Id g_shader_id;
        Id g_geometry_layout_id;
        Id g_uniform_set_id;
        Id g_depth_config_id;
        Id g_blend_config_id;
        DrawKey g_draw_key;
    }

    // ============================================================= //
    // ============================================================= //

    AtlasImage::AtlasImage(ks::Object::Key const &key,
                           Scene* scene,
                           shared_ptr<Widget> parent,
                           shared_ptr<ImageAtlas> image_atlas,
                           Id atlas_image_id) :
        DrawableWidget(key,scene,parent),
        m_cmlist_draw_data(
            m_scene->GetDrawSystem()->
            GetDrawDataComponentList()),
        m_image_atlas(image_atlas),
        m_atlas_image_id(atlas_image_id)
    {}

    void AtlasImage::Init(ks::Object::Key const &,
              shared_ptr<AtlasImage> const &)
    {
        setupTypeInit(m_scene);

        this->createDrawables();
    }

    AtlasImage::~AtlasImage()
    {
        destroyDrawables();
    }

    void AtlasImage::onWidthChanged()
    {
        m_upd_geometry = true;

        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateDrawables;
    }

    void AtlasImage::onHeightChanged()
    {
        m_upd_geometry = true;

        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateDrawables;
    }

    void AtlasImage::onClipIdUpdated()
    {
        auto& draw_data = m_cmlist_draw_data->GetComponent(m_entity_id);
        draw_data.key.SetClip(m_clip_id);
    }

    void AtlasImage::onTransformUpdated()
    {
        m_upd_geometry = true;

        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateDrawables;
    }

    void AtlasImage::onAccOpacityUpdated()
    {
        m_upd_geometry = true;

        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateDrawables;
    }

    void AtlasImage::createDrawables()
    {
        // Destroy drawable resources if they already exist
        if(m_scene->GetEntityHasComponents<DrawData>(m_entity_id))
        {
            destroyDrawables();
        }

        // Get TextureSet from Atlas
        g_draw_key.SetTextureSet(
                    m_image_atlas->GetImage(m_atlas_image_id).
                    texture_set_id);

        // DrawData
        m_cmlist_draw_data->Create(
                    m_entity_id,
                    DrawData{
                        g_draw_key,
                        make_unique<std::vector<u8>>(),
                        visible.Get()
                    });

        // UpdateData
        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateDrawables;

        m_upd_geometry = true;
    }

    void AtlasImage::destroyDrawables()
    {
        m_cmlist_draw_data->Remove(m_entity_id);
    }

    void AtlasImage::updateDrawables()
    {
        if(m_upd_geometry)
        {
            updateGeometry();
            m_upd_geometry = false;
        }
    }

    void AtlasImage::updateGeometry()
    {
        auto& draw_data = m_cmlist_draw_data->
                GetComponent(m_entity_id);

        auto& list_vx = draw_data.vx_buffer;
        list_vx->clear();
        list_vx->reserve(6*sizeof(Vertex));

        auto const w = width.Get();
        auto const h = height.Get();
        float const z = 0.0f;
        float const n = 0.0f;
        float const this_opacity = m_accumulated_opacity;

        auto const &xf = m_cmlist_xf_data->
                GetComponent(m_entity_id).world_xf;

        // Get proper texture coords
        auto image_desc = m_image_atlas->GetImage(m_atlas_image_id);
//            image_desc.s0 = 0;
//            image_desc.s1 = 1;
//            image_desc.t0 = 0;
//            image_desc.t1 = 1;

        // tl
        ks::gl::Buffer::PushElement<Vertex>(
                    *list_vx,
                    Vertex{
                        xf*glm::vec4(n,n,z,1),
                        glm::vec3{image_desc.s0,image_desc.t0,this_opacity}
                    });

        // tl
        ks::gl::Buffer::PushElement<Vertex>(
                    *list_vx,
                    Vertex{
                        xf*glm::vec4(n,n,z,1),
                        glm::vec3{image_desc.s0,image_desc.t0,this_opacity}
                    });

        // bl
        ks::gl::Buffer::PushElement<Vertex>(
                    *list_vx,
                    Vertex{
                        xf*glm::vec4(n,h,z,1),
                        glm::vec3{image_desc.s0,image_desc.t1,this_opacity}
                    });

        // tr
        ks::gl::Buffer::PushElement<Vertex>(
                    *list_vx,
                    Vertex{
                        xf*glm::vec4(w,n,z,1),
                        glm::vec3{image_desc.s1,image_desc.t0,this_opacity}
                    });

        // br
        ks::gl::Buffer::PushElement<Vertex>(
                    *list_vx,
                    Vertex{
                        xf*glm::vec4(w,h,z,1),
                        glm::vec3{image_desc.s1,image_desc.t1,this_opacity}
                    });

        // br
        ks::gl::Buffer::PushElement<Vertex>(
                    *list_vx,
                    Vertex{
                        xf*glm::vec4(w,h,z,1),
                        glm::vec3{image_desc.s1,image_desc.t1,this_opacity}
                    });
    }

    void AtlasImage::setupTypeInit(Scene* scene)
    {
        static_assert(sizeof(Vertex) == 28,
                      "ERROR: Vertex struct has padding");

        static bool init = false;
        if(!init)
        {
            auto init_callback = [&,scene]()
            {
                auto draw_system = scene->GetDrawSystem();

                // Add the Shader
                g_shader_id =
                        draw_system->RegisterShader(
                            "AtlasImage",
                            raintk_atlas_image_vert_glsl,
                            raintk_atlas_image_frag_glsl);

                // Register the geometry layout
                g_geometry_layout_id =
                        draw_system->RegisterGeometryLayout(
                            g_geometry_layout);

                // Create common uniform set
                auto uniform_set = make_shared<ks::draw::UniformSet>();
                uniform_set->list_uniforms.push_back(
                            make_shared<ks::gl::Uniform<GLint>>(
                                "u_s_tex0",0));

                g_uniform_set_id =
                        draw_system->RegisterUniformSet(
                            uniform_set);

                // Setup raster configs
                g_depth_config_id =
                        draw_system->RegisterDepthConfig(
                            [](ks::gl::StateSet* state_set){
                                state_set->SetDepthTest(GL_TRUE);
                                state_set->SetDepthMask(GL_FALSE);
                            });

                g_blend_config_id =
                        draw_system->RegisterBlendConfig(
                            [](ks::gl::StateSet* state_set){
                                state_set->SetBlend(GL_TRUE);
                                state_set->SetBlendFunction(
                                    GL_ONE,
                                    GL_ONE_MINUS_SRC_ALPHA,
                                    GL_ONE,
                                    GL_ONE_MINUS_SRC_ALPHA);
                            });

                // Setup DrawKeys
                g_draw_key.SetShader(g_shader_id);
                g_draw_key.SetGeometryLayout(g_geometry_layout_id);
                g_draw_key.SetUniformSet(g_uniform_set_id);
                g_draw_key.SetTransparency(true);
                g_draw_key.SetDepthConfig(g_depth_config_id);
                g_draw_key.SetBlendConfig(g_blend_config_id);
                g_draw_key.SetPrimitive(ks::gl::Primitive::TriangleStrip);
            };

            init_callback();
            DrawableWidget::SetInitCallback<AtlasImage>(init_callback);

            init = true;
        }
    }

    // ============================================================= //
    // ============================================================= //
}
