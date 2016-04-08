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

#include <raintk/RainTkText.hpp>
#include <raintk/RainTkScene.hpp>
#include <raintk/RainTkDrawSystem.hpp>
#include <raintk/RainTkTransformSystem.hpp>

namespace raintk
{
    // ============================================================= //
    // ============================================================= //

    namespace text_detail
    {
        // Shader
        #include <raintk/shaders/text_sdf_glsl_empirical.hpp>
//         #include <raintk/shaders/text_sdf_glsl.hpp>
//         #include <raintk/shaders/text_sdf_glsl_es_no_ext.hpp>

        // VertexLayout
        using AttrType = ks::gl::VertexBuffer::Attribute::Type;

        ks::gl::VertexLayout const g_vx_layout {
            { "a_v2_position", AttrType::Float, 2, false },
            { "a_v3_tex0_index", AttrType::Float, 3, false }
        };

        shared_ptr<GeometryLayout> g_geometry_layout(
                new GeometryLayout{
                    g_vx_layout,
                    sizeof(Vertex),
                    4096*sizeof(Vertex)*6 // buffer size in bytes
                });

        // Draw setup
        Id g_shader_id;
        Id g_geometry_layout_id;
        Id g_depth_config_id;
        Id g_blend_config_id;
        DrawKey g_base_draw_key;


        // Helper
        template<typename T>
        void OrderedUniqueInsert(std::vector<T>& list_data,T ins_data)
        {
            // lower bound == first value thats greater than or equal
            auto it = std::lower_bound(list_data.begin(),
                                       list_data.end(),
                                       ins_data);

            // insert when: it==end, *it != ins_data (ins_data is greater)
            if((it==list_data.end()) || (*it != ins_data) ) {
                list_data.insert(it,ins_data);
            }
        }

        // The length of each UniformArray in a given batch
        uint const k_batch_array_size=16;


        std::map<Id,std::vector<GlyphBatchAvail>> g_lkup_glyph_batch_avail;


        void CreateTextUniformBatch(
                DrawSystem* draw_system,
                std::vector<GlyphBatchAvail>& list_batches)
        {
            // Create a new TextUniformBatch
            list_batches.emplace_back();
            auto& text_batch = list_batches.back();

            text_batch.uniform_set =
                    make_shared<ks::draw::UniformSet>();

            text_batch.uniform_set->list_uniforms.push_back(
                        make_shared<ks::gl::UniformArray<glm::mat4>>(
                            "u_array_m4_model",
                            std::vector<glm::mat4>(k_batch_array_size)
                            )
                        );

            text_batch.uniform_set->list_uniforms.push_back(
                        make_shared<ks::gl::UniformArray<glm::vec4>>(
                            "u_array_v4_color",
                            std::vector<glm::vec4>(k_batch_array_size)
                            )
                        );

            text_batch.uniform_set->list_uniforms.push_back(
                        make_shared<ks::gl::UniformArray<float>>(
                            "u_array_f_res",
                            std::vector<float>(k_batch_array_size)
                            )
                        );

            text_batch.uniform_set->list_uniforms.push_back(
                        make_shared<ks::gl::Uniform<GLint>>(
                            "u_s_tex0",0));

            text_batch.uniform_set_id =
                    draw_system->RegisterUniformSet(
                        text_batch.uniform_set);

            text_batch.list_avail.reserve(k_batch_array_size);
            for(uint i=0; i < k_batch_array_size; i++)
            {
                text_batch.list_avail.push_back(i);
            }
        }


        GlyphBatchIndex AcquireGlyphBatchIndex(DrawSystem* draw_system,
                                               Id texture_set_id)
        {
            auto it = g_lkup_glyph_batch_avail.find(texture_set_id);
            if(it == g_lkup_glyph_batch_avail.end())
            {
                // Create new TextUniformBatch
                it = g_lkup_glyph_batch_avail.emplace(
                            texture_set_id,
                            std::vector<GlyphBatchAvail>()).first;
            }


            // Try and find an existing Batch with available space
            GlyphBatchIndex glyph_batch_index;

            auto& list_batches = it->second;
            for(auto& batch : list_batches)
            {
                if(!batch.list_avail.empty())
                {
                    glyph_batch_index.uniform_set = batch.uniform_set;
                    glyph_batch_index.uniform_set_id = batch.uniform_set_id;
                    glyph_batch_index.index = batch.list_avail.back();
                    batch.list_avail.pop_back();

                    return glyph_batch_index;
                }
            }


            // Create a new Batch if no space is available
            CreateTextUniformBatch(draw_system,it->second);

            auto& batch = it->second.back();
            glyph_batch_index.uniform_set = batch.uniform_set;
            glyph_batch_index.uniform_set_id = batch.uniform_set_id;
            glyph_batch_index.index = batch.list_avail.back();
            batch.list_avail.pop_back();

            return glyph_batch_index;
        }

        void ReleaseGlyphBatchIndex(DrawSystem* draw_system,
                                    Id texture_set_id,
                                    Id uniform_set_id,
                                    uint index)
        {
            (void)draw_system;

            auto it = g_lkup_glyph_batch_avail.find(texture_set_id);

            auto& list_batches = it->second;
            for(auto& batch : list_batches)
            {
                if(batch.uniform_set_id == uniform_set_id)
                {
                    batch.list_avail.push_back(index);

                    // TODO Is it worth removing this TextUniformBatch
                    // if list_avail is full?
                }
            }
        }
    }

    // ============================================================= //
    // ============================================================= //

    using namespace text_detail;

    Text::Text(ks::Object::Key const &key,
         shared_ptr<Widget> parent,
         std::string name) :
        DrawableWidget(key,parent,name),
        m_cmlist_draw_data(
            m_scene->GetDrawSystem()->
            GetDrawDataComponentList())
    {
        m_nz_glyph_tl.a_v2_position.x = 0;
        m_nz_glyph_tl.a_v2_position.y = 0;

        m_nz_glyph_br.a_v2_position.x = 0;
        m_nz_glyph_br.a_v2_position.y = 0;
    }

    void Text::Init(ks::Object::Key const &,
              shared_ptr<Text> const &this_text)
    {
        setupTypeInit(m_scene);

        // Set TextHint defaults
        m_text_hint.font_search = ks::text::Hint::FontSearch::Explicit;
        m_text_hint.direction = ks::text::Hint::Direction::LeftToRight;
        m_text_hint.script = ks::text::Hint::Script::Single;

        // Connect properties
        m_cid_color =
                color.signal_changed.Connect(
                    this_text,
                    &Text::onColorChanged,
                    ks::ConnectionType::Direct);

        m_cid_text =
                text.signal_changed.Connect(
                    this_text,
                    &Text::onTextChanged,
                    ks::ConnectionType::Direct);

        m_cid_font =
                font.signal_changed.Connect(
                    this_text,
                    &Text::onFontChanged,
                    ks::ConnectionType::Direct);

        m_cid_size =
                size.signal_changed.Connect(
                    this_text,
                    &Text::onSizeChanged,
                    ks::ConnectionType::Direct);

        m_cid_line_width =
                line_width.signal_changed.Connect(
                    this_text,
                    &Text::onLineWidthChanged,
                    ks::ConnectionType::Direct);

        m_cid_alignment =
                alignment.signal_changed.Connect(
                    this_text,
                    &Text::onAlignmentChanged,
                    ks::ConnectionType::Direct);
    }

    Text::~Text()
    {
        destroyDrawables();
    }

    ks::text::Hint& Text::GetTextHint()
    {
        return m_text_hint;
    }

    void Text::onOpacityChanged()
    {
        m_upd_color = true;

        auto& upd_data = m_cmlist_update_data->GetComponent(m_entity_id);
        upd_data.update |= UpdateData::UpdateDrawables;
    }

    void Text::onVisibilityChanged()
    {
        for(auto& glyph_batch : m_list_glyph_batches)
        {
            auto& draw_data =
                    m_cmlist_draw_data->GetComponent(
                        glyph_batch.entity_id);

            draw_data.visible = visible.Get();
        }
    }

    void Text::onClipIdUpdated()
    {
        for(auto& glyph_batch : m_list_glyph_batches)
        {
            auto& draw_data =
                    m_cmlist_draw_data->GetComponent(
                        glyph_batch.entity_id);

            draw_data.key.SetClip(m_clip_id);
        }
    }

    void Text::onTransformUpdated()
    {
        m_upd_xf = true;

        auto& upd_data = m_cmlist_update_data->GetComponent(m_entity_id);
        upd_data.update |= UpdateData::UpdateDrawables;
    }

    void Text::onColorChanged()
    {
        m_upd_color = true;

        auto& upd_data = m_cmlist_update_data->GetComponent(m_entity_id);
        upd_data.update |= UpdateData::UpdateDrawables;
    }

    void Text::onTextChanged()
    {
        m_u16_text = ks::text::TextManager::ConvertStringUTF8ToUTF16(text.Get());

        auto& upd_data = m_cmlist_update_data->GetComponent(m_entity_id);
        upd_data.update |= UpdateData::UpdateWidget;
    }

    void Text::onFontChanged()
    {
        auto new_text_hint =
                m_scene->GetTextManager()->
                CreateHint(font.Get());

        m_text_hint.list_prio_fonts =
                new_text_hint.list_prio_fonts;

        auto& upd_data = m_cmlist_update_data->GetComponent(m_entity_id);
        upd_data.update |= UpdateData::UpdateWidget;
    }

    void Text::onSizeChanged()
    {
        auto& upd_data = m_cmlist_update_data->GetComponent(m_entity_id);
        upd_data.update |= UpdateData::UpdateWidget;
    }

    void Text::onLineWidthChanged()
    {
        auto& upd_data = m_cmlist_update_data->GetComponent(m_entity_id);
        upd_data.update |= UpdateData::UpdateWidget;
    }

    void Text::onAlignmentChanged()
    {
        auto& upd_data = m_cmlist_update_data->GetComponent(m_entity_id);
        upd_data.update |= UpdateData::UpdateWidget;
    }

    void Text::update()
    {
        // Calculate dimensions

        // The max line width must be scaled by the
        // size of the text but watch out for the
        // float->uint cast overflow
        float scaled_line_width =
                line_width.Get()*
                (m_scene->GetTextGlyphSizePx()/size.Get());

        if(scaled_line_width < float(k_max_line_width))
        {
            m_text_hint.max_line_width_px =
                    uint(scaled_line_width);
        }

        // Get/generate the glyphs from the TextManager
        m_list_lines =
                m_scene->GetTextManager()->GetGlyphs(
                    m_u16_text,
                    m_text_hint);

        auto& list_lines = *m_list_lines;

        float new_height = 0.0f;
        float new_width = 0.0f;

        // The first baseline is 'ascent' pixels below the top
        float baseline_y = list_lines[0].ascent;

        for(auto const &line : list_lines)
        {
            new_width = std::max<float>(new_width,line.x_max);
            baseline_y += line.spacing;
        }

        new_height =
                baseline_y -
                list_lines.back().spacing +
                abs(list_lines.back().descent);

        float const k_scale =
                size.Get()/m_scene->GetTextGlyphSizePx();

        this->height = (new_height*k_scale);
        this->width = (new_width*k_scale);

        // Indicate drawables must be updated
        m_upd_recreate = true;
        auto& upd_data = m_cmlist_update_data->GetComponent(m_entity_id);
        upd_data.update |= UpdateData::UpdateDrawables;
    }

    // Should only be called from createDrawables/removeDrawables
    void Text::releaseBatches()
    {
        for(auto& glyph_batch : m_list_glyph_batches)
        {
            auto& draw_data =
                    m_cmlist_draw_data->GetComponent(
                        glyph_batch.entity_id);

            ReleaseGlyphBatchIndex(
                        m_scene->GetDrawSystem(),
                        draw_data.key.GetTextureSet(),
                        draw_data.key.GetUniformSet(),
                        glyph_batch.index);

            m_scene->RemoveEntity(glyph_batch.entity_id);
        }

        m_list_glyph_batches.clear();
    }

    // Should only be called from createDrawables/removeDrawables
    void Text::acquireBatches()
    {
        // We need to call GetGlyphs to determine which
        // atlases are required

        if(m_list_lines==nullptr)
        {
            // The max line width must be scaled by the
            // size of the text but watch out for the
            // float->uint cast overflow
            float scaled_line_width =
                    line_width.Get()*
                    (m_scene->GetTextGlyphSizePx()/size.Get());

            if(scaled_line_width < float(k_max_line_width))
            {
                m_text_hint.max_line_width_px =
                        uint(scaled_line_width);
            }

            // Get/generate the glyphs from the TextManager
            m_list_lines =
                    m_scene->GetTextManager()->GetGlyphs(
                        m_u16_text,
                        m_text_hint);
        }

        auto& list_lines = *m_list_lines;

        // Find required atlas indices
        std::vector<uint> list_atlas_idxs;
        for(auto const &line : list_lines)
        {
            for(auto atlas : line.list_atlases)
            {
                OrderedUniqueInsert<uint>(list_atlas_idxs,atlas);
            }
        }

        // Acquire a GlyphBatchIndex for each required atlas
        std::vector<GlyphBatchIndex> list_glyph_batch_indices(
                    list_atlas_idxs.back()+1);

        auto const & lkup_text_atlas_data =
                m_scene->GetTextAtlasData();

        for(auto atlas_idx : list_atlas_idxs)
        {
            list_glyph_batch_indices[atlas_idx] =
                    AcquireGlyphBatchIndex(
                        m_scene->GetDrawSystem(),
                        lkup_text_atlas_data.at(atlas_idx)->
                        texture_set_id);
        }

        // Create GlyphBatches and corresponding entities
        for(auto atlas_idx : list_atlas_idxs)
        {
            auto const ent_id = m_scene->CreateEntity();

            // Save GlyphBatch
            m_list_glyph_batches.emplace_back(
                        GlyphBatch{
                            ent_id,
                            list_glyph_batch_indices[atlas_idx].uniform_set,
                            list_glyph_batch_indices[atlas_idx].index,
                            atlas_idx,
                            list_glyph_batch_indices[atlas_idx].uniform_set_id,
                            lkup_text_atlas_data.at(atlas_idx)->texture_set_id
                        });
        }
    }

    // Assumes that m_list_lines is valid
    void Text::createDrawables()
    {
        if(m_list_glyph_batches.empty())
        {
            acquireBatches();
        }

        auto& list_lines = *m_list_lines;

        // Find required atlas indices and total glyph count
        std::vector<uint> list_atlas_idxs;
        for(auto const &line : list_lines)
        {
            for(auto atlas : line.list_atlases)
            {
                OrderedUniqueInsert<uint>(list_atlas_idxs,atlas);
            }
        }

        // Acquire a GlyphBatchIndex for each required atlas
        std::vector<GlyphBatch*> list_glyph_batches(
                    list_atlas_idxs.back()+1,nullptr);

        std::vector<UPtrBuffer> list_glyph_vx_buffs(
                    list_atlas_idxs.back()+1);

        for(auto atlas_idx : list_atlas_idxs)
        {
            for(auto& glyph_batch : m_list_glyph_batches)
            {
                if(glyph_batch.atlas_index == atlas_idx)
                {
                    list_glyph_batches[atlas_idx] = &glyph_batch;
                }
            }

            list_glyph_vx_buffs[atlas_idx] =
                    make_unique<std::vector<u8>>();
        }

        // Generate glyph vertex buffers
        genGlyphVertexBuffers(
                    list_lines,
                    list_glyph_batches,
                    list_glyph_vx_buffs);

        findNonZeroGlyph(list_lines);

        // Create DrawData and TransformData for each batch

        auto xf_data_copy =
                m_cmlist_xf_data->
                GetComponent(m_entity_id);

        for(auto& batch : m_list_glyph_batches)
        {
            m_cmlist_xf_data->Create(
                        batch.entity_id,
                        xf_data_copy);

            auto& draw_data =
                    m_cmlist_draw_data->Create(
                        batch.entity_id,
                        DrawData{
                            g_base_draw_key,
                            std::move(list_glyph_vx_buffs[batch.atlas_index]),
                            visible.Get()
                        });

            draw_data.key.SetClip(m_clip_id);
            draw_data.key.SetTextureSet(batch.texture_set_id);
            draw_data.key.SetUniformSet(batch.uniform_set_id);
        }

        updateTransformUniforms();
        updateColorUniforms();

        m_upd_xf = false;
        m_upd_color = false;

        m_list_lines = nullptr;
    }

    void Text::destroyDrawables()
    {
        releaseBatches();
    }

    void Text::updateDrawables()
    {
        if(m_upd_recreate)
        {
            destroyDrawables();
            createDrawables();
        }
        else
        {
            if(m_upd_xf)
            {
                updateTransformUniforms();
            }

            if(m_upd_color)
            {
                updateColorUniforms();
            }
        }

        m_upd_recreate = false;
        m_upd_xf = false;
        m_upd_color = false;
    }

    void Text::updateColorUniforms()
    {
        for(auto &batch : m_list_glyph_batches)
        {
            auto const &this_color = color.Get();

            glm::vec4 const color_norm(
                        this_color.r/255.0f,
                        this_color.g/255.0f,
                        this_color.b/255.0f,
                        opacity.Get());

            auto u_array_v4_color =
                    static_cast<ks::gl::UniformArray<glm::vec4>*>(
                        batch.uniform_set->list_uniforms[1].get());

            u_array_v4_color->Update(
                        color_norm,
                        batch.index);
        }
    }

    void Text::updateTransformUniforms()
    {
        auto const& widget_xf_data =
                m_cmlist_xf_data->
                GetComponent(m_entity_id);

        for(auto& batch : m_list_glyph_batches)
        {
            auto& xf_data =
                    m_cmlist_xf_data->
                    GetComponent(batch.entity_id);

            xf_data = widget_xf_data;

            auto& world_xf = xf_data.world_xf;

            // Model transform
            auto u_array_m4_model =
                    static_cast<ks::gl::UniformArray<glm::mat4>*>(
                        batch.uniform_set->list_uniforms[0].get());

            u_array_m4_model->Update(
                        world_xf,
                        batch.index);

            // Glyph texture res
            float res = 0.0;
            float glyph_width_px =
                    (m_nz_glyph_br.a_v2_position.x-
                     m_nz_glyph_tl.a_v2_position.x);

            if(glyph_width_px != 0.0f)
            {
                // Apply world xf to get the final glyph dims
                auto world_glyph_br = world_xf*glm::vec4(m_nz_glyph_br.a_v2_position,0,1);
                auto world_glyph_tl = world_xf*glm::vec4(m_nz_glyph_tl.a_v2_position,0,1);

                glyph_width_px =
                        world_glyph_br.x -
                        world_glyph_tl.x;

                float tex_coord_width =
                        m_nz_glyph_br.a_v3_tex0_index.x -
                        m_nz_glyph_tl.a_v3_tex0_index.x;

                res = tex_coord_width/glyph_width_px;
            }

            auto u_array_f_res =
                    static_cast<ks::gl::UniformArray<float>*>(
                        batch.uniform_set->list_uniforms[2].get());

            u_array_f_res->Update(res,batch.index);
        }
    }

    void Text::genGlyphVertexBuffers(
            std::vector<ks::text::Line> const &list_lines,
            std::vector<GlyphBatch*> const &list_glyph_batches,
            std::vector<UPtrBuffer>& list_glyph_vx_buffs)
    {
        // Texture scaling factor
        float const k_div_atlas =
                1.0f/m_scene->GetTextAtlasSizePx();

        // Glyph scaling factor
        float const k_glyph =
                size.Get()/m_scene->GetTextGlyphSizePx();

        float text_width = 0.0f;

        // The first baseline is 'ascent' pixels below the top
        float baseline_y = list_lines[0].ascent;


        for(auto const &line : list_lines)
        {
            text_width = std::max<float>(text_width,line.x_max);
        }

        // Get alignment and shift
        auto this_alignment = alignment.Get();
        if(this_alignment == Alignment::Auto)
        {
            if(list_lines[0].rtl)
            {
                this_alignment = Alignment::Right;
            }
            else
            {
                this_alignment = Alignment::Left;
            }
        }

        std::array<float,4> list_alignment_shifts;

        for(auto const &line : list_lines)
        {
            // Calculate alignment shifts
            list_alignment_shifts[1] = 0.0f; // Left
            list_alignment_shifts[2] = text_width-line.x_max; // Right
            list_alignment_shifts[3] = list_alignment_shifts[2]*0.5f; // Center

            float const alignment_shift =
                    list_alignment_shifts[
                        static_cast<uint>(this_alignment)];

            for(ks::text::Glyph const &glyph : line.list_glyphs)
            {
                // Original glyph dimensions (without any SDF
                // borders)
                uint const o_glyph_width = glyph.x1-glyph.x0;
                uint const o_glyph_height= glyph.y1-glyph.y0;

                auto& list_vx = list_glyph_vx_buffs[glyph.atlas];


                // Do I really need this check? Would it be
                // cheaper to leave it out?
                if(o_glyph_width==0 || o_glyph_height==0)
                {
                    continue;
                }

                uint const glyph_width = o_glyph_width + (2*glyph.sdf_x);
                uint const glyph_height= o_glyph_height + (2*glyph.sdf_y);

                float x0 = (glyph.x0-glyph.sdf_x + alignment_shift)*k_glyph;
                float x1 = (glyph.x1+glyph.sdf_x + alignment_shift)*k_glyph;
                float y0 = (baseline_y-(glyph.y0-glyph.sdf_y))*k_glyph;
                float y1 = (baseline_y-(glyph.y1+glyph.sdf_y))*k_glyph;

                float s0 = glyph.tex_x*k_div_atlas;
                float s1 = (glyph.tex_x+glyph_width)*k_div_atlas;

                // tex_y must be flipped
                float t0 = glyph.tex_y*k_div_atlas;
                float t1 = (glyph.tex_y+glyph_height)*k_div_atlas;

                float uniform_index =
                        static_cast<float>(
                            list_glyph_batches[glyph.atlas]->index);

                // BL
                ks::gl::Buffer::PushElement<Vertex>(
                            *list_vx,
                            Vertex{
                                glm::vec2{x0,y0},
                                glm::vec3{s0,t1,uniform_index}
                            });

                // TR
                ks::gl::Buffer::PushElement<Vertex>(
                            *list_vx,
                            Vertex{
                                glm::vec2{x1,y1},
                                glm::vec3{s1,t0,uniform_index}
                            });

                // TL
                ks::gl::Buffer::PushElement<Vertex>(
                            *list_vx,
                            Vertex{
                                glm::vec2{x0,y1},
                                glm::vec3{s0,t0,uniform_index}
                            });

                // BL
                ks::gl::Buffer::PushElement<Vertex>(
                            *list_vx,
                            Vertex{
                                glm::vec2{x0,y0},
                                glm::vec3{s0,t1,uniform_index}
                            });

                // BR
                ks::gl::Buffer::PushElement<Vertex>(
                            *list_vx,
                            Vertex{
                                glm::vec2{x1,y0},
                                glm::vec3{s1,t1,uniform_index}
                            });

                // TR
                ks::gl::Buffer::PushElement<Vertex>(
                            *list_vx,
                            Vertex{
                                glm::vec2{x1,y1},
                                glm::vec3{s1,t0,uniform_index}
                            });
            }

            // Move baseline down to next line
            baseline_y += line.spacing;
        }
    }


    void Text::findNonZeroGlyph(std::vector<ks::text::Line> const &list_lines)
    {
        // Texture scaling factor
        float const k_div_atlas =
                1.0f/m_scene->GetTextAtlasSizePx();

        // Glyph scaling factor
        float const k_glyph =
                size.Get()/m_scene->GetTextGlyphSizePx();

        for(auto const &line : list_lines)
        {
            float baseline_y = list_lines[0].ascent;

            for(auto const &glyph : line.list_glyphs)
            {
                // Original glyph dimensions (without any SDF
                // borders)
                uint const o_glyph_width = glyph.x1-glyph.x0;
                uint const o_glyph_height= glyph.y1-glyph.y0;

                if(o_glyph_width==0 || o_glyph_height==0)
                {
                    continue;
                }

                uint const glyph_width = o_glyph_width + (2*glyph.sdf_x);
                uint const glyph_height= o_glyph_height + (2*glyph.sdf_y);

                float x0 = (glyph.x0-glyph.sdf_x)*k_glyph;
                float x1 = (glyph.x1+glyph.sdf_x)*k_glyph;
                float y0 = (baseline_y-(glyph.y0-glyph.sdf_y))*k_glyph;
                float y1 = (baseline_y-(glyph.y1+glyph.sdf_y))*k_glyph;

                float s0 = glyph.tex_x*k_div_atlas;
                float s1 = (glyph.tex_x+glyph_width)*k_div_atlas;

                // tex_y must be flipped
                float t0 = glyph.tex_y*k_div_atlas;
                float t1 = (glyph.tex_y+glyph_height)*k_div_atlas;

                m_nz_glyph_tl =
                        Vertex{
                            glm::vec2{x0,y1},
                            glm::vec3{s0,t0,0}
                        };

                m_nz_glyph_br =
                        Vertex{
                            glm::vec2{x1,y0},
                            glm::vec3{s1,t1,0}
                        };

                return;
            }
        }

        // If no non-zero glyphs exist, set the dims to zero
        m_nz_glyph_tl.a_v2_position.x = 0;
        m_nz_glyph_tl.a_v2_position.y = 0;

        m_nz_glyph_br.a_v2_position.x = 0;
        m_nz_glyph_br.a_v2_position.y = 0;
    }

    void Text::setupTypeInit(Scene* scene)
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

                std::string mod_text_sdf_vert_glsl =
                        text_sdf_vert_glsl;

                std::string const replace_search =
                        "#define K_NUM_U_ARRAY_SIZE 0";

                auto search_pos =
                        mod_text_sdf_vert_glsl.find(
                            replace_search);

                mod_text_sdf_vert_glsl.replace(
                            (search_pos+replace_search.size()-1),1,
                            ks::ToString(k_batch_array_size));

                g_shader_id =
                        draw_system->RegisterShader(
                            "image",
                            mod_text_sdf_vert_glsl,
                            text_sdf_frag_glsl);

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
                                    GL_SRC_ALPHA,
                                    GL_ONE_MINUS_SRC_ALPHA,
                                    GL_SRC_ALPHA,
                                    GL_ONE_MINUS_SRC_ALPHA);
                            });

                // Setup DrawKeys
                g_base_draw_key.SetShader(g_shader_id);
                g_base_draw_key.SetGeometryLayout(g_geometry_layout_id);
                g_base_draw_key.SetTransparency(true);
                g_base_draw_key.SetDepthConfig(g_depth_config_id);
                g_base_draw_key.SetBlendConfig(g_blend_config_id);
                g_base_draw_key.SetPrimitive(ks::gl::Primitive::Triangles);
            };

            init_callback();
            DrawableWidget::SetInitCallback<Text>(init_callback);

            init = true;
        }
    }


    // ============================================================= //
    // ============================================================= //
}
