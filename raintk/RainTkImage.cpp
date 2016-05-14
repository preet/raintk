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

#include <ks/shared/KsImage.hpp>
#include <ks/draw/KsDrawRenderSystem.hpp>

#include <raintk/RainTkDrawSystem.hpp>
#include <raintk/RainTkScene.hpp>
#include <raintk/RainTkImage.hpp>

namespace raintk
{
    // ============================================================= //
    // ============================================================= //

    namespace
    {
        #include <raintk/shaders/RainTkImage.glsl.hpp>

        struct Vertex
        {
            glm::vec4 a_v4_position; // 16
            glm::vec2 a_v2_tex0; // 8
        }; // sizeof == 24

        ks::gl::VertexLayout const g_vx_layout {
            { "a_v4_position", AttrType::Float, 4, false },
            { "a_v2_tex0", AttrType::Float, 2, false }
        };

        shared_ptr<GeometryLayout> g_geometry_layout(
            new GeometryLayout{
            g_vx_layout,
            sizeof(Vertex),
            24*6*128 // buffer size in bytes
        });

        // Uniform set
        shared_ptr<ks::draw::UniformSet> g_uniform_set;

        // Setup
        Id g_shader_id;
        Id g_geometry_layout_id;
        Id g_depth_config_id;
        Id g_blend_config_id;
        DrawKey g_draw_key;

        // ============================================================= //

        Image::Source const g_default_image_source{
            Image::Source::Lifetime::Permanent,
            [](){
                ks::Image<ks::RGBA8> image(3,3);

                image.GetData().push_back(ks::RGBA8{255,0,0,255});
                image.GetData().push_back(ks::RGBA8{255,128,0,255});
                image.GetData().push_back(ks::RGBA8{255,255,0,255});

                image.GetData().push_back(ks::RGBA8{0,255,0,255});
                image.GetData().push_back(ks::RGBA8{0,255,128,255});
                image.GetData().push_back(ks::RGBA8{0,255,255,255});

                image.GetData().push_back(ks::RGBA8{0,0,255,255});
                image.GetData().push_back(ks::RGBA8{128,0,255,255});
                image.GetData().push_back(ks::RGBA8{255,0,255,255});

                return shared_ptr<ks::ImageData>(
                            image.ConvertToImageDataPtr().release());
            }
        };

    }

    // ============================================================= //
    // ============================================================= //

    Image::Image(ks::Object::Key const &key,
                 Scene* scene,
                 shared_ptr<Widget> parent) :
        DrawableWidget(key,scene,parent),
        m_cmlist_draw_data(
            m_scene->GetDrawSystem()->
            GetDrawDataComponentList()),
        m_uniform_set_id(0),
        m_texture_set_id(0),
        m_image_data_size_px(1,1),
        m_image_data(nullptr),
        m_upd_recreate(false),
        m_upd_geometry(false),
        m_upd_xf(false),
        m_upd_texture(false),
        m_upd_tile_ratio(false),
        m_upd_smooth(false)
    {
        // Set a default initial source
        source = g_default_image_source;
    }

    void Image::Init(ks::Object::Key const &,
                     shared_ptr<Image> const &this_image)
    {
        setupTypeInit(m_scene);

        this->createDrawables();

        m_cid_source =
                source.signal_changed.Connect(
                    this_image,
                    &Image::onSourceChanged,
                    ks::ConnectionType::Direct);

        m_cid_fill_mode =
                fill_mode.signal_changed.Connect(
                    this_image,
                    &Image::onFillModeChanged,
                    ks::ConnectionType::Direct);
    }

    Image::~Image()
    {
        destroyDrawables();
    }

    void Image::onWidthChanged()
    {
        m_upd_geometry = true;
        m_upd_tile_ratio = true;

        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateDrawables;
    }

    void Image::onHeightChanged()
    {
        m_upd_geometry = true;
        m_upd_tile_ratio = true;

        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateDrawables;
    }

    void Image::onClipIdUpdated()
    {
        auto& draw_data = m_cmlist_draw_data->GetComponent(m_entity_id);
        draw_data.key.SetClip(m_clip_id);
    }

    void Image::onTransformUpdated()
    {
        m_upd_xf = true;

        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateDrawables;
    }

    void Image::onOpacityChanged()
    {
        auto u_f_opacity =
                static_cast<ks::gl::Uniform<float>*>(
                    m_uniform_set->list_uniforms[2].get());

        u_f_opacity->Update(opacity.Get());
    }

    void Image::onSourceChanged()
    {
        m_upd_texture = true;
        m_upd_tile_ratio = true;

        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateDrawables;
    }

    void Image::onFillModeChanged()
    {
        m_upd_tile_ratio = true;

        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateDrawables;
    }

    void Image::onSmoothChanged()
    {
        m_upd_smooth = true;

        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateDrawables;
    }

    void Image::createDrawables()
    {
        // Destroy drawable resources fi they already exist
        if(m_scene->GetEntityHasComponents<DrawData>(m_entity_id))
        {
            destroyDrawables();
        }

        // UniformSet
        m_uniform_set = make_shared<ks::draw::UniformSet>();

        m_uniform_set->list_uniforms.push_back(
                    make_unique<ks::gl::Uniform<glm::mat4>>(
                        "u_m4_model",
                        m_cmlist_xf_data->GetComponent(m_entity_id).world_xf));

        m_uniform_set->list_uniforms.push_back(
                    make_unique<ks::gl::Uniform<glm::vec2>>(
                        "u_v2_tile",glm::vec2{1.0,1.0}));

        m_uniform_set->list_uniforms.push_back(
                    make_unique<ks::gl::Uniform<float>>(
                        "u_f_opacity",1.0));

        m_uniform_set->list_uniforms.push_back(
                    make_unique<ks::gl::Uniform<GLint>>(
                        "u_s_tex0",0));

        m_uniform_set_id =
                m_scene->GetDrawSystem()->
                RegisterUniformSet(m_uniform_set);

        // TextureSet
        m_texture_set = make_shared<ks::draw::TextureSet>();

        auto texture =
                make_shared<ks::gl::Texture2D>(
                    ks::gl::Texture2D::Format::RGBA8);

        texture->SetWrapModes(
                    ks::gl::Texture::Wrap::ClampToEdge,
                    ks::gl::Texture::Wrap::ClampToEdge);

        m_texture_set->list_texture_desc.emplace_back(
                    std::move(texture),0); // 0 = tex unit

        m_texture_set_id =
                m_scene->GetDrawSystem()->
                RegisterTextureSet(m_texture_set);

        // Key
        auto draw_key = g_draw_key;
        draw_key.SetUniformSet(m_uniform_set_id);
        draw_key.SetTextureSet(m_texture_set_id);

        // DrawData
        m_cmlist_draw_data->Create(
                    m_entity_id,
                    DrawData{
                        draw_key,
                        make_unique<std::vector<u8>>(),
                        visible.Get()
                    });

        // UpdateData
        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateDrawables;

        m_upd_geometry = true;
        m_upd_xf = true;
        m_upd_texture = true;
        m_upd_tile_ratio = true;
        m_upd_smooth = true;
    }

    void Image::destroyDrawables()
    {
        m_cmlist_draw_data->Remove(m_entity_id);

        m_scene->GetDrawSystem()->RemoveUniformSet(m_uniform_set_id);
        m_scene->GetDrawSystem()->RemoveTextureSet(m_texture_set_id);

        m_uniform_set_id = 0;
        m_uniform_set = nullptr;

        m_texture_set_id = 0;
        m_texture_set = nullptr;
    }

    void Image::updateDrawables()
    {
        if(m_upd_recreate)
        {
            destroyDrawables();
            createDrawables();
        }
        if(m_upd_geometry)
        {
            updateGeometry();
        }
        if(m_upd_xf)
        {
            updateTransform();
        }
        if(m_upd_texture)
        {
            updateTexture();
        }
        if(m_upd_tile_ratio)
        {
            updateTileRatio();
        }
        if(m_upd_smooth)
        {
            updateSmooth();
        }

        m_upd_recreate = false;
        m_upd_geometry = false;
        m_upd_xf = false;
        m_upd_texture = false;
        m_upd_tile_ratio = false;
        m_upd_smooth = false;
    }

    void Image::updateGeometry()
    {
        auto& draw_data = m_cmlist_draw_data->GetComponent(m_entity_id);
        auto& list_vx = draw_data.vx_buffer;
        list_vx->clear();
        list_vx->reserve(6*sizeof(Vertex));

        auto const w = width.Get();
        auto const h = height.Get();
        float const z = 0.0f;
        float const n = 0.0f;

        // bl
        ks::gl::Buffer::PushElement<Vertex>(
                    *list_vx,
                    Vertex{
                        glm::vec4{n,h,z,1},
                        glm::vec2{0,1}
                    });

        // tr
        ks::gl::Buffer::PushElement<Vertex>(
                    *list_vx,
                    Vertex{
                        glm::vec4{w,n,z,1},
                        glm::vec2{1,0}
                    });

        // tl
        ks::gl::Buffer::PushElement<Vertex>(
                    *list_vx,
                    Vertex{
                        glm::vec4{n,n,z,1},
                        glm::vec2{0,0}
                    });

        // bl
        ks::gl::Buffer::PushElement<Vertex>(
                    *list_vx,
                    Vertex{
                        glm::vec4{n,h,z,1},
                        glm::vec2{0,1}
                    });

        // br
        ks::gl::Buffer::PushElement<Vertex>(
                    *list_vx,
                    Vertex{
                        glm::vec4{w,h,z,1},
                        glm::vec2{1,1}
                    });

        // tr
        ks::gl::Buffer::PushElement<Vertex>(
                    *list_vx,
                    Vertex{
                        glm::vec4{w,n,z,1},
                        glm::vec2{1,0}
                    });
    }

    void Image::updateTransform()
    {
        auto u_m4_model =
                static_cast<ks::gl::Uniform<glm::mat4>*>(
                    m_uniform_set->list_uniforms[0].get());

        u_m4_model->Update(
                    m_cmlist_xf_data->
                    GetComponent(m_entity_id).world_xf);
    }

    void Image::updateTexture()
    {
        auto const filter = smooth.Get() ?
                    ks::gl::Texture2D::Filter::Linear :
                    ks::gl::Texture2D::Filter::Nearest;

        if(source.Get().lifetime == Source::Lifetime::Permanent)
        {
            auto image_data = source.Get().load();
            auto& texture = m_texture_set->list_texture_desc[0].first;

            m_image_data_size_px.x = image_data->width;
            m_image_data_size_px.y = image_data->height;

            texture->UpdateTexture(
                        ks::gl::Texture2D::Update{
                            ks::gl::Texture2D::Update::ReUpload,
                            glm::u16vec2(0,0),
                            image_data
                        });

            texture->SetFilterModes(filter,filter);
        }
        else
        {
            // Load the source if we haven't already
            if(source.Get().load)
            {
                m_image_data = source.Get().load();
                m_image_data_size_px.x = m_image_data->width;
                m_image_data_size_px.y = m_image_data->height;
                source.Get().load = nullptr;
            }

            auto& texture = m_texture_set->list_texture_desc[0].first;

            // We hang on to the image data if the source is
            // temporary in case we need to re upload it
            texture->UpdateTexture(
                        ks::gl::Texture2D::Update{
                            ks::gl::Texture2D::Update::ReUpload,
                            glm::u16vec2(0,0),
                            m_image_data
                        });

            texture->SetFilterModes(filter,filter);
        }
    }

    void Image::updateTileRatio()
    {
        auto u_v2_tile =
                static_cast<ks::gl::Uniform<glm::vec2>*>(
                    m_uniform_set->list_uniforms[1].get());

        if(fill_mode.Get() == FillMode::Stretch)
        {
            u_v2_tile->Update(glm::vec2{1.0f,1.0f});
        }
        else
        {
            u_v2_tile->Update(
                        glm::vec2{
                            width.Get()/m_image_data_size_px.x,
                            height.Get()/m_image_data_size_px.y
                        });
        }
    }

    void Image::updateSmooth()
    {
        auto& texture = m_texture_set->list_texture_desc[0].first;
        auto const filter = smooth.Get() ?
                    ks::gl::Texture2D::Filter::Linear :
                    ks::gl::Texture2D::Filter::Nearest;

        texture->SetFilterModes(filter,filter);
    }

    void Image::setupTypeInit(Scene* scene)
    {
        static_assert(sizeof(Vertex) == 24,
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
                            "image",
                            image_vert_glsl,
                            image_frag_glsl);

                // Register the geometry layout
                g_geometry_layout_id =
                        draw_system->RegisterGeometryLayout(
                            g_geometry_layout);

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
                g_draw_key.SetTransparency(true);
                g_draw_key.SetDepthConfig(g_depth_config_id);
                g_draw_key.SetBlendConfig(g_blend_config_id);
                g_draw_key.SetPrimitive(ks::gl::Primitive::Triangles);
            };

            init_callback();
            DrawableWidget::SetInitCallback<Image>(init_callback);

            init = true;
        }
    }
}
