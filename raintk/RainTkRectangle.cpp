/*
   Copyright (C) 2015-2016 Preet Desai (preet.desai@gmail.com)

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

#include <raintk/RainTkRectangle.hpp>
#include <raintk/RainTkScene.hpp>
#include <raintk/RainTkTransformSystem.hpp>
#include <raintk/RainTkDrawSystem.hpp>

namespace raintk
{
    // ============================================================= //
    // ============================================================= //

    namespace
    {
        #include <raintk/shaders/color_attr_glsl.hpp>

        struct Vertex
        {
            glm::vec4 a_v4_position;
            glm::u8vec4 a_v4_color;
        }; // 20 bytes

        shared_ptr<GeometryLayout> g_geometry_layout(
            new GeometryLayout{
            ks::gl::VertexLayout
            {
                {
                    "a_v4_position",
                    AttrType::Float,
                    4,
                    false
                },
                {
                    "a_v4_color",
                    AttrType::UByte,
                    4,
                    true
                }
            },
            sizeof(Vertex),
            50000 // buffer size: 50000 bytes
        });

        Id g_shader_id;
        Id g_geometry_layout_id;
        Id g_depth_config_id_opq;
        Id g_blend_config_id_opq;
        Id g_depth_config_id_xpr;
        Id g_blend_config_id_xpr;
        DrawKey g_draw_key_opq;
        DrawKey g_draw_key_xpr;
    }

    // ============================================================= //
    // ============================================================= //

    Rectangle::Rectangle(ks::Object::Key const &key,
                         shared_ptr<Widget> parent,
                         std::string name) :
        DrawableWidget(key,parent,name),
        m_cmlist_draw_data(
            m_scene->GetDrawSystem()->
            GetDrawDataComponentList())
    {
        // Reasonable dimension defaults
        width = mm(25.0f);
        height = mm(25.0f);
    }

    void Rectangle::Init(ks::Object::Key const &,
                         shared_ptr<Rectangle> const &this_rect)
    {
        setupTypeInit(m_scene);

        this->createDrawables();

        // Connect properties
        m_cid_color =
                color.signal_changed.Connect(
                    this_rect,
                    &Rectangle::onColorChanged,
                    ks::ConnectionType::Direct);
    }

    Rectangle::~Rectangle()
    {
        destroyDrawables();
    }

    void Rectangle::onWidthChanged()
    {
        auto& upd_data = m_cmlist_update_data->GetComponent(m_entity_id);
        upd_data.update |= UpdateData::UpdateDrawables;
    }

    void Rectangle::onHeightChanged()
    {
        auto& upd_data = m_cmlist_update_data->GetComponent(m_entity_id);
        upd_data.update |= UpdateData::UpdateDrawables;
    }

    void Rectangle::onOpacityChanged()
    {
        auto new_opacity = opacity.Get();
        auto& draw_data = m_cmlist_draw_data->GetComponent(m_entity_id);

        if(new_opacity < 1.0f)
        {
            draw_data.key = g_draw_key_xpr;
        }
        else
        {
            draw_data.key = g_draw_key_opq;
        }
    }

    void Rectangle::onVisibilityChanged()
    {
        auto& draw_data = m_cmlist_draw_data->GetComponent(m_entity_id);
        draw_data.visible = visible.Get();
    }

    void Rectangle::onClipIdUpdated()
    {
        auto& draw_data = m_cmlist_draw_data->GetComponent(m_entity_id);
        draw_data.key.SetClip(m_clip_id);
    }

    void Rectangle::onTransformUpdated()
    {
        auto& upd_data = m_cmlist_update_data->GetComponent(m_entity_id);
        upd_data.update |= UpdateData::UpdateDrawables;
    }

    void Rectangle::onColorChanged()
    {
        auto& upd_data = m_cmlist_update_data->GetComponent(m_entity_id);
        upd_data.update |= UpdateData::UpdateDrawables;
    }

    void Rectangle::createDrawables()
    {
        // Destroy DrawData if it already exists
        // (is this really necessary?)
        if(m_scene->GetEntityHasComponents<DrawData>(m_entity_id))
        {
            destroyDrawables();
        }

        // Create new DrawData
        auto draw_key =
                (opacity.Get() < 1.0f) ?
                    g_draw_key_xpr : g_draw_key_opq;

        m_cmlist_draw_data->Create(
                    m_entity_id,
                    DrawData{
                        draw_key,
                        make_unique<std::vector<u8>>(),
                        visible.Get()});

        // UpdateData
        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateDrawables;
    }

    void Rectangle::destroyDrawables()
    {
        m_cmlist_draw_data->Remove(m_entity_id);
    }

    void Rectangle::updateDrawables()
    {
        updateGeometry();
    }

    void Rectangle::updateGeometry()
    {
        auto& draw_data = m_cmlist_draw_data->GetComponent(m_entity_id);
        auto& list_vx = draw_data.vx_buffer;

        list_vx->clear();
        list_vx->reserve(6*sizeof(Vertex));

        auto const o = opacity.Get();
        glm::vec3 rgb = color.Get();

        glm::u8vec4 const c {
            static_cast<u8>(o*rgb.r),
            static_cast<u8>(o*rgb.g),
            static_cast<u8>(o*rgb.b),
            static_cast<u8>(o*255)
        };

        auto const w = width.Get();
        auto const h = height.Get();
        float const z = 0.0f;
        float const n = 0.0f;
        auto const &xf = m_cmlist_xf_data->GetComponent(m_entity_id).world_xf;

        // The first and last vertices are doubled to
        // introduce degenerate triangles in the triangle
        // strip. This way multiple disjoint Rectangles
        // can be merged and drawn in a single batch.

        // tl
        ks::gl::Buffer::PushElement<Vertex>(
                    *list_vx,Vertex{xf*glm::vec4(n,n,z,1),c});

        // tl
        ks::gl::Buffer::PushElement<Vertex>(
                    *list_vx,Vertex{xf*glm::vec4(n,n,z,1),c});

        // bl
        ks::gl::Buffer::PushElement<Vertex>(
                    *list_vx,Vertex{xf*glm::vec4(n,h,z,1),c});

        // tr
        ks::gl::Buffer::PushElement<Vertex>(
                    *list_vx,Vertex{xf*glm::vec4(w,n,z,1),c});

        // br
        ks::gl::Buffer::PushElement<Vertex>(
                    *list_vx,Vertex{xf*glm::vec4(w,h,z,1),c});

        // br
        ks::gl::Buffer::PushElement<Vertex>(
                    *list_vx,Vertex{xf*glm::vec4(w,h,z,1),c});
    }

    void Rectangle::setupTypeInit(Scene* scene)
    {
        static_assert(sizeof(Vertex) == 20,
                      "ERROR: Vertex struct has padding");

        static bool init = false;
        if(!init)
        {
            auto init_callback =
                    [&,scene]()
                    {
                        auto draw_system = scene->GetDrawSystem();

                        // Add the Shader
                        g_shader_id =
                                draw_system->RegisterShader(
                                    "flat_color_attr",
                                    color_attr_vert_glsl,
                                    color_attr_frag_glsl);

                        // Register the geometry layout
                        g_geometry_layout_id =
                                draw_system->RegisterGeometryLayout(
                                    g_geometry_layout);

                        // Setup raster configs
                        g_depth_config_id_opq =
                                draw_system->RegisterDepthConfig(
                                    [](ks::gl::StateSet* state_set){
                                        state_set->SetDepthMask(GL_TRUE);
                                        state_set->SetDepthTest(GL_TRUE);
                                    });

                        g_blend_config_id_opq =
                                draw_system->RegisterBlendConfig(
                                    [](ks::gl::StateSet* state_set){
                                        state_set->SetBlend(GL_FALSE);
                                    });

                        g_depth_config_id_xpr =
                                draw_system->RegisterDepthConfig(
                                    [](ks::gl::StateSet* state_set){
                                        state_set->SetDepthTest(GL_TRUE);
                                        state_set->SetDepthMask(GL_FALSE);
                                    });

                        g_blend_config_id_xpr =
                                draw_system->RegisterBlendConfig(
                                    [](ks::gl::StateSet* state_set){
                                        state_set->SetBlend(GL_TRUE);
                                        state_set->SetBlendFunction(
                                            GL_SRC_ALPHA,
                                            GL_ONE_MINUS_SRC_ALPHA,
                                            GL_SRC_ALPHA,
                                            GL_ONE_MINUS_SRC_ALPHA);
                                    });

                        // Setup DrawKeys
                        g_draw_key_opq.SetShader(g_shader_id);
                        g_draw_key_opq.SetGeometryLayout(g_geometry_layout_id);
                        g_draw_key_opq.SetTransparency(false);
                        g_draw_key_opq.SetDepthConfig(g_depth_config_id_opq);
                        g_draw_key_opq.SetBlendConfig(g_blend_config_id_opq);
                        g_draw_key_opq.SetPrimitive(ks::gl::Primitive::TriangleStrip);

                        g_draw_key_xpr.SetShader(g_shader_id);
                        g_draw_key_xpr.SetGeometryLayout(g_geometry_layout_id);
                        g_draw_key_xpr.SetTransparency(true);
                        g_draw_key_xpr.SetDepthConfig(g_depth_config_id_xpr);
                        g_draw_key_xpr.SetBlendConfig(g_blend_config_id_xpr);
                        g_draw_key_xpr.SetPrimitive(ks::gl::Primitive::TriangleStrip);
                    };

            init_callback();
            DrawableWidget::SetInitCallback<Rectangle>(init_callback);

            init = true;
        }
    }

    // ============================================================= //
}
