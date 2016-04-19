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

#include <raintk/RainTkDrawSystem.hpp>
#include <raintk/RainTkScene.hpp>
#include <raintk/RainTkLog.hpp>
#include <raintk/RainTkWidget.hpp>
#include <raintk/RainTkDrawableWidget.hpp>

namespace raintk
{
    namespace draw_debug
    {
        #include <raintk/shaders/color_attr_glsl.hpp>

        using AttrType = ks::gl::VertexBuffer::Attribute::Type;

        struct Vertex
        {
            glm::vec4 a_v4_position;
            glm::u8vec4 a_v4_color;
        };

        UPtrBuffer CreateLine(glm::vec4 v4_a,
                              glm::vec4 v4_b,
                              glm::u8vec4 color=glm::u8vec4(255,255,0,255))
        {
            glm::vec3 a(v4_a);
            glm::vec3 b(v4_b);

            glm::vec3 ab_dirn = b-a;
            glm::vec3 ab_norm(ab_dirn.y*-1.0f,ab_dirn.x,0.0); // 2d perp
//            ab_norm = glm::normalize(ab_norm)*mm(0.25f);
            ab_norm = glm::normalize(ab_norm)*px(1.0f);

            // ('left' and 'right' are arbitrary here)
            glm::vec4 tl(a+ab_norm,1.0);
            glm::vec4 bl(b+ab_norm,1.0);
            glm::vec4 tr(a-ab_norm,1.0);
            glm::vec4 br(b-ab_norm,1.0);

            UPtrBuffer vx_buffer_line;
            vx_buffer_line = make_unique<std::vector<u8>>();


            // The first and last vertices are doubled to introduce
            // degenerate triangles in the triangle strip
            ks::gl::Buffer::PushElement<Vertex>(
                        *vx_buffer_line,
                        Vertex{
                            tl,color});

            ks::gl::Buffer::PushElement<Vertex>(
                        *vx_buffer_line,
                        Vertex{
                            tl,color});

            ks::gl::Buffer::PushElement<Vertex>(
                        *vx_buffer_line,
                        Vertex{
                            bl,color});

            ks::gl::Buffer::PushElement<Vertex>(
                        *vx_buffer_line,
                        Vertex{
                            tr,color});

            ks::gl::Buffer::PushElement<Vertex>(
                        *vx_buffer_line,
                        Vertex{
                            br,color});

            ks::gl::Buffer::PushElement<Vertex>(
                        *vx_buffer_line,
                        Vertex{
                            br,color});

            return vx_buffer_line;
        }
    }

    // ============================================================= //

    using RenderData = ks::draw::RenderData<DrawKey>;

    using RenderDataComponentList =
        ks::ecs::ComponentList<SceneKey,RenderData>;

    using VertexBufferAllocator = ks::draw::VertexBufferAllocator;

    using IndexBufferAllocator = ks::draw::IndexBufferAllocator;

    // ============================================================= //

    namespace
    {
        bool CalcBoundingBoxOverlap(BoundingBox const &a,
                                    BoundingBox const &b)
        {
            bool outside =
                    (a.x0 > b.x1) ||
                    (a.x1 < b.x0) ||
                    (a.y0 > b.y1) ||
                    (a.y1 < b.y0);

            return (!outside);
        }

        BoundingBox CalcBoundingBoxIntersection(BoundingBox const &a,
                                                BoundingBox const &b)
        {
            bool const outside =
                    (a.x0 > b.x1) ||
                    (a.x1 < b.x0) ||
                    (a.y0 > b.y1) ||
                    (a.y1 < b.y0);

            if(outside)
            {
                return BoundingBox{0.0f,0.0f,0.0f,0.0f};
            }

            return BoundingBox{
                std::max(a.x0,b.x0),
                std::max(a.y0,b.y0),
                std::min(a.x1,b.x1),
                std::min(a.y1,b.y1)
            };
        }

        std::vector<std::vector<DrawData*>>
        CreateCommonKeyGroups(
                std::vector<DrawData> &list_draw_data,
                std::vector<Id> const &list_draw_data_ids)
        {
            std::vector<std::vector<DrawData*>> list_groups;

            auto it_start = list_draw_data_ids.begin();
            auto it_end = list_draw_data_ids.begin();

            while(it_end != list_draw_data_ids.end())
            {
                if(list_draw_data[*it_start].key.GetKey() !=
                   list_draw_data[*it_end].key.GetKey())
                {
                    list_groups.emplace_back();

                    for(auto it = it_start; it != it_end; ++it)
                    {
                        list_groups.back().push_back(&(list_draw_data[*it]));
                    }

                    it_start = it_end;
                }
                ++it_end;
            }

            // Save the last group
            list_groups.emplace_back();

            for(auto it = it_start; it != it_end; ++it)
            {
                list_groups.back().push_back(&(list_draw_data[*it]));
            }


            return list_groups;
        }

        std::vector<std::vector<DrawData*>>
        CreateMergedGroupsForDrawData(
                std::vector<DrawData*> &list_draw_data,
                ks::draw::BufferLayout const * buffer_layout)
        {
            std::vector<std::vector<DrawData*>> list_grouped_draw_data(1);

            uint const vx_block_size =
                    buffer_layout->GetVertexBufferAllocator(0)->
                    GetBlockSize();

            uint vx_block_used=0;

            for(auto draw_data : list_draw_data)
            {
                uint const single_vx_buff_size =
                        draw_data->vx_buffer->size();

                vx_block_used += single_vx_buff_size;

                if(vx_block_used > vx_block_size)
                {
                    if(vx_block_size < single_vx_buff_size)
                    {
                        throw ks::Exception(
                                    ks::Exception::ErrorLevel::ERROR,
                                    "DrawSystem: Geometry size exceeds "
                                    "BufferAllocator block size");
                    }

                    // Create a new single geometry list
                    list_grouped_draw_data.emplace_back();
                    vx_block_used = single_vx_buff_size;
                }

                list_grouped_draw_data.back().push_back(draw_data);
            }

            return list_grouped_draw_data;
        }


        // We use list_clip_ids instead of list_draw_data because
        // we don't have to do a potentially relatively expensive
        // check (Widget::GetIsDrawable) before writing to DrawData
        void AssignClipIds(Widget* widget,
                           std::vector<TransformData>& list_xf_data,
                           std::vector<BoundingBox>& clip_stack,
                           std::vector<BoundingBox>& list_final_clips)
        {
            auto const has_clip = widget->clip.Get();
            auto const ent_id = widget->GetEntityId();

            if(has_clip)
            {
                // Create a new clip bounding box by intersecting
                // with the closest clip on the stack

                auto& xf_data = list_xf_data[ent_id];

                auto this_clip =
                        CalcBoundingBoxIntersection(
                            clip_stack.back(),
                            xf_data.bbox);

                clip_stack.push_back(this_clip);
                list_final_clips.push_back(clip_stack.back());
                widget->SetClipId(list_final_clips.size()-1);
            }
            else
            {
                // Inherit the clip id of the parent
                widget->SetClipId(widget->GetParent()->GetClipId());
            }

            for(auto& child : widget->GetChildren())
            {
                AssignClipIds(
                            child.get(),
                            list_xf_data,
                            clip_stack,
                            list_final_clips);
            }

            if(has_clip)
            {
                clip_stack.pop_back();
            }
        }
    }

    // ============================================================= //

    DrawSystem::DrawSystem(Scene* scene, RenderSystem* render_system) :
        m_scene(scene),
        m_render_system(render_system),
        m_cmlist_upd_data(
            static_cast<UpdateDataComponentList*>(
                m_scene->template GetComponentList<UpdateData>())),
        m_cmlist_xf_data(
            static_cast<TransformDataComponentList*>(
                m_scene->template GetComponentList<TransformData>())),
        m_show_bboxes(false)
    {
        // Create the DrawData component list
        m_scene->template RegisterComponentList<DrawData>(
                    make_unique<DrawDataComponentList>(*m_scene));

        m_cmlist_draw_data =
                static_cast<DrawDataComponentList*>(
                    m_scene->template GetComponentList<DrawData>());

        // Reserve index 0 for gm_layout (indicates invalid)
        m_list_gm_layouts.Add(nullptr);


        setupDebugRendering();
    }

    DrawSystem::~DrawSystem()
    {

    }

    std::string DrawSystem::GetDesc() const
    {
        return "raintk DrawSystem";
    }

    std::vector<BoundingBox> const & DrawSystem::GetClipRegions() const
    {
        return m_list_clip_regions;
    }

    std::vector<Id> DrawSystem::GetOpaqueDrawOrderList() const
    {
        return m_list_opq_render_ent_ids;
    }

    std::vector<Id> DrawSystem::GetTransparentDrawOrderList() const
    {
        return m_list_xpr_render_ent_ids;
    }

    DrawDataComponentList* DrawSystem::GetDrawDataComponentList() const
    {
        return m_cmlist_draw_data;
    }

    void DrawSystem::SetClippingEnabled(bool enabled)
    {
        m_clipping_enabled = enabled;
    }

    void DrawSystem::SetShowBoundingBoxes(bool show_bboxes)
    {
        m_show_bboxes = show_bboxes;
    }

    void DrawSystem::SetShowClipOutlines(bool show_clip_outlines)
    {
        m_show_clip_outlines = show_clip_outlines;
    }

    Id DrawSystem::RegisterGeometryLayout(
            shared_ptr<GeometryLayout const> gm_layout)
    {

        uint const k_max_buffer_size_bytes = (1024*1024*2);

        if(gm_layout->buffer_size_hint_bytes==0)
        {
            throw ks::Exception(
                        ks::Exception::ErrorLevel::ERROR,
                        "DrawSystem: RegisterGeometryLayout: "
                        "Invalid buffer size hint: "
                        "Must be greater than 0");
        }
        else if(gm_layout->buffer_size_hint_bytes > k_max_buffer_size_bytes)
        {
            throw ks::Exception(
                        ks::Exception::ErrorLevel::ERROR,
                        "DrawSystem: RegisterGeometryLayout: "
                        "Invalid buffer size hint: "
                        "Must be less than "+
                        ks::ToString(k_max_buffer_size_bytes));
        }

        // Create a corresponding ks::draw::BufferLayout
        std::vector<shared_ptr<ks::draw::VertexBufferAllocator>> list_vx_allocators;
        shared_ptr<ks::draw::IndexBufferAllocator> ix_allocator;

        list_vx_allocators.push_back(
                    make_shared<VertexBufferAllocator>(
                        gm_layout->buffer_size_hint_bytes));

        Id const new_id = m_list_gm_layouts.Add(gm_layout);

        m_lkup_gm_buffer_layouts.emplace(
                    new_id,
                    make_unique<ks::draw::BufferLayout>(
                        ks::gl::Buffer::Usage::Stream,
                        std::vector<ks::gl::VertexLayout>{gm_layout->vx_layout},
                        list_vx_allocators,
                        ix_allocator)); // Currently unused

        return new_id;
    }

    void DrawSystem::RemoveGeometryLayout(Id gm_layout_id)
    {
        m_lkup_gm_buffer_layouts.erase(gm_layout_id);
        m_list_gm_layouts.Remove(gm_layout_id);
    }

    Id DrawSystem::RegisterShader(std::string shader_desc,
                      std::string shader_source_vsh,
                      std::string shader_source_fsh)
    {
        return m_render_system->RegisterShader(
                    shader_desc,
                    shader_source_vsh,
                    shader_source_fsh);
    }

    void DrawSystem::RemoveShader(Id shader_id)
    {
        m_render_system->RemoveShader(shader_id);
    }

    Id DrawSystem::RegisterDepthConfig(ks::draw::StateSetCb depth_config)
    {
        return m_render_system->RegisterDepthConfig(depth_config);
    }

    void DrawSystem::RemoveDepthConfig(Id depth_config_id)
    {
        m_render_system->RemoveDepthConfig(depth_config_id);
    }

    Id DrawSystem::RegisterBlendConfig(ks::draw::StateSetCb blend_config)
    {
        return m_render_system->RegisterBlendConfig(blend_config);
    }

    void DrawSystem::RemoveBlendConfig(Id blend_config_id)
    {
        m_render_system->RemoveBlendConfig(blend_config_id);
    }

    Id DrawSystem::RegisterStencilConfig(ks::draw::StateSetCb stencil_config)
    {
        return m_render_system->RegisterStencilConfig(stencil_config);
    }

    void DrawSystem::RemoveStencilConfig(Id stencil_config_id)
    {
        m_render_system->RemoveStencilConfig(stencil_config_id);
    }

    Id DrawSystem::RegisterTextureSet(shared_ptr<ks::draw::TextureSet> texture_set)
    {
        return m_render_system->RegisterTextureSet(texture_set);
    }

    void DrawSystem::RemoveTextureSet(Id texture_set_id)
    {
        m_render_system->RemoveTextureSet(texture_set_id);
    }

    Id DrawSystem::RegisterUniformSet(shared_ptr<ks::draw::UniformSet> uniform_set)
    {
        return m_render_system->RegisterUniformSet(uniform_set);
    }

    void DrawSystem::RemoveUniformSet(Id uniform_set_id)
    {
        m_render_system->RemoveUniformSet(uniform_set_id);
    }

    void DrawSystem::Update(TimePoint const &/*prev_time*/,
                            TimePoint const &/*curr_time*/)
    {
        // Remove old RenderData for DrawData entities
        for(auto ent_id : m_list_opq_render_ent_ids)
        {
            m_scene->RemoveEntity(ent_id);
        }

        for(auto ent_id : m_list_xpr_render_ent_ids)
        {
            m_scene->RemoveEntity(ent_id);
        }

        m_list_opq_render_ent_ids.clear();
        m_list_xpr_render_ent_ids.clear();


        // Remove old RenderData for bounding box debug
        if(m_show_bboxes || m_show_clip_outlines)
        {
            // Remove old bounding box entities
            for(auto ent_id : m_list_debug_ent_ids)
            {
                m_scene->RemoveEntity(ent_id);
            }
            m_list_debug_ent_ids.clear();

            if(m_show_bboxes)
            {
                createBoundingBoxDrawData();
            }

            if(m_show_clip_outlines)
            {
                createClipOutlineDrawData();
            }
        }


        // Assign clip ids
        if(m_clipping_enabled)
        {
            auto root_widget = m_scene->GetRootWidget().get();

            auto& list_xf_data =
                    m_cmlist_xf_data->GetSparseList();

            std::vector<BoundingBox> clip_stack;
            clip_stack.push_back(
                        m_cmlist_xf_data->GetComponent(
                            root_widget->GetEntityId()).bbox);

            m_list_clip_regions.clear();
            m_list_clip_regions.push_back(clip_stack.back());

            AssignClipIds(root_widget,
                          list_xf_data,
                          clip_stack,
                          m_list_clip_regions);
        }


        // Get the current list of entities with DrawData and store
        // them in lists separated by transparency / opaque
        auto &list_entities = m_scene->GetEntityList();

        auto const updatable_mask =
                m_scene->template GetComponentMask<
                    UpdateData>();

        auto const drawable_mask =
                m_scene->template GetComponentMask<
                    TransformData,DrawData>();

        auto& list_upd_data =
                m_cmlist_upd_data->GetSparseList();

        auto& list_draw_data =
                m_cmlist_draw_data->GetSparseList();

        std::vector<Id> list_opq_draw_data_ids;
        std::vector<Id> list_xpr_draw_data_ids;

        // Update the DrawableWidget if required. This should
        // happen in its own pass through all the entities because
        // DrawableWidget::updateDrawables may create or destroy
        // entities and components. This needs to happen before
        // the drawable_mask entities are collected.

        // TODO Should creating or destroying entities in
        // DrawableWidget::updateDrawables even be allowed?
        // It can lead to delayed updates for example if a
        // TransformData is created from updateDrawables

        for(uint ent_id=0; ent_id < list_entities.size(); ent_id++)
        {
            // Update the DrawableWidget if required
            if((list_entities[ent_id].mask & updatable_mask) == updatable_mask)
            {
                auto& upd_data = list_upd_data[ent_id];

                if(upd_data.update & UpdateData::UpdateDrawables)
                {
                    DrawableWidget* drawable_widget =
                            static_cast<DrawableWidget*>(upd_data.widget);

                    drawable_widget->updateDrawables();
                    upd_data.update &= ~(UpdateData::UpdateDrawables);
                }
            }
        }

        // Save drawable entities into opaque / transparent id lists
        // NOTE: See above comments for why we have two separate
        // loops through all entities
        for(uint ent_id=0; ent_id < list_entities.size(); ent_id++)
        {
            if((list_entities[ent_id].mask & drawable_mask) == drawable_mask)
            {
                auto& draw_data = list_draw_data[ent_id];

                if(draw_data.visible)
                {
                    if(draw_data.key.GetTransparency())
                    {
                        list_xpr_draw_data_ids.push_back(ent_id);
                    }
                    else
                    {
                        list_opq_draw_data_ids.push_back(ent_id);
                    }
                }
            }
        }

        // Sort DrawData lists so that they can be grouped
        sortIntoOpaqueGroups(list_opq_draw_data_ids);
        sortIntoTransparencyGroups(list_xpr_draw_data_ids);

        // Create grouped lists from the sorted DrawData and
        // create RenderData entities for the grouped DrawData
        if(!list_opq_draw_data_ids.empty())
        {
            auto list_grouped_opq_draw_data =
                    CreateCommonKeyGroups(
                        list_draw_data,
                        list_opq_draw_data_ids);

            createRenderDataForCommonKeyGroups(
                        ks::draw::Transparency::Opaque,
                        list_grouped_opq_draw_data,
                        m_list_opq_render_ent_ids);
        }

        if(!list_xpr_draw_data_ids.empty())
        {
            auto list_grouped_xpr_draw_data =
                    CreateCommonKeyGroups(
                        list_draw_data,
                        list_xpr_draw_data_ids);

            createRenderDataForCommonKeyGroups(
                        ks::draw::Transparency::Transparent,
                        list_grouped_xpr_draw_data,
                        m_list_xpr_render_ent_ids);
        }
    }

    void DrawSystem::sortIntoOpaqueGroups(
            std::vector<Id> &list_opq_draw_data_ids)
    {
        auto& list_xf_data =
                m_cmlist_xf_data->GetSparseList();

        auto& list_draw_data =
                m_cmlist_draw_data->GetSparseList();

        // Sort primarily by key to minimize state changes,
        // and then by depth within the same key such that
        // objects in front are drawn first
        auto sort_opq =
                [&list_draw_data,&list_xf_data](Id a, Id b)
                {
                    if(!(list_draw_data[a].key == list_draw_data[b].key))
                    {
                        auto const depth_a = list_xf_data[a].world_xf[3].z;
                        auto const depth_b = list_xf_data[b].world_xf[3].z;
                        return (depth_a > depth_b);
                    }
                    return (list_draw_data[a].key < list_draw_data[b].key);
                };

        std::sort(list_opq_draw_data_ids.begin(),
                  list_opq_draw_data_ids.end(),
                  sort_opq);
    }

    void DrawSystem::sortIntoTransparencyGroups(
            std::vector<Id> &list_xpr_draw_data_ids)
    {
        auto& list_xf_data =
                m_cmlist_xf_data->GetSparseList();

        auto& list_draw_data =
                m_cmlist_draw_data->GetSparseList();

        auto sort_xpr =
                [&list_draw_data,&list_xf_data](Id a, Id b)
        {
            auto const depth_a = list_xf_data[a].world_xf[3].z;
            auto const depth_b = list_xf_data[b].world_xf[3].z;

            if(depth_a != depth_b)
            {
                return (depth_a < depth_b);
            }

            return (list_draw_data[a].key < list_draw_data[b].key);
        };

        std::stable_sort(list_xpr_draw_data_ids.begin(),
                         list_xpr_draw_data_ids.end(),
                         sort_xpr);
    }

    void DrawSystem::createRenderDataForCommonKeyGroups(
            ks::draw::Transparency transparency,
            std::vector<std::vector<DrawData*>> &list_common_key_groups,
            std::vector<Id>& list_render_ent_ids)
    {
        for(auto& common_key_group : list_common_key_groups)
        {
            auto const &key = common_key_group[0]->key;

            auto buffer_layout =
                    m_lkup_gm_buffer_layouts[
                        key.GetGeometryLayout()].get();

            // Split the common key group into even more lists based
            // on the size of the geometry buffer block size
            auto list_merged_gm_draw_data =
                    CreateMergedGroupsForDrawData(
                        common_key_group,
                        buffer_layout);

            for(auto& merged_gm_draw_data : list_merged_gm_draw_data)
            {
                createRenderDataForDrawData(
                            key,
                            buffer_layout,
                            merged_gm_draw_data,
                            transparency,
                            list_render_ent_ids);
            }
        }
    }

    void DrawSystem::createRenderDataForDrawData(
            DrawKey const &key,
            ks::draw::BufferLayout const * buffer_layout,
            std::vector<DrawData*> const &list_draw_data,
            ks::draw::Transparency transparency,
            std::vector<Id>& list_render_ent_ids)
    {
        // Create an entity and its merged RenderData
        auto cmlist_render_data =
                m_render_system->
                GetRenderDataComponentList();

        auto render_ent_id = m_scene->CreateEntity();

        auto& render_data =
                cmlist_render_data->Create(
                    render_ent_id,
                    key,
                    buffer_layout,
                    nullptr,
                    std::vector<u8>{static_cast<u8>(m_scene->GetMainDrawStageId())},
                    transparency);

        auto& merged_gm = render_data.GetGeometry();

        // NOTE: We set retain geometry to false because we recreate
        //       RenderData every time DrawData is updated. This also
        //       ensures that if DrawSystem->Update is called multiple
        //       times without a corresponding Sync, we don't have to
        //       keep the corresponding Geometry alive until the Sync
        //       occurs.

        merged_gm.SetRetainGeometry(false);

        merged_gm.GetVertexBuffers().
                push_back(make_unique<std::vector<u8>>());

        auto& merged_vx_buffer = merged_gm.GetVertexBuffer(0);

        // TODO: use memcpy here instead to speed this up
        for(auto draw_data : list_draw_data)
        {
            merged_vx_buffer->insert(
                        merged_vx_buffer->end(),
                        draw_data->vx_buffer->begin(),
                        draw_data->vx_buffer->end());
        }

        if(merged_vx_buffer->empty())
        {
            m_scene->RemoveEntity(render_ent_id);
        }
        else
        {
            merged_gm.SetAllUpdated();

            // Save RenderData entity
            list_render_ent_ids.push_back(render_ent_id);
        }
    }

    void DrawSystem::createClipOutlineDrawData()
    {
        // Get widget list
        std::vector<Widget*> stack_widgets;
        stack_widgets.push_back(m_scene->GetRootWidget().get());

        while(!stack_widgets.empty())
        {
            auto widget = stack_widgets.back();
            stack_widgets.pop_back();

            auto const &poly_vx =
                    m_cmlist_xf_data->
                    GetComponent(widget->GetEntityId()).poly_vx;

            UPtrBuffer poly_vx_buffer =
                    make_unique<std::vector<u8>>();

            for(uint i=0; i < poly_vx.size(); i++)
            {
                auto const &vx0 = poly_vx[i];
                auto const &vx1 = poly_vx[(i+1)%poly_vx.size()];

                auto line_buffer =
                        raintk::draw_debug::CreateLine(
                            glm::vec4(vx0.x,vx0.y,0,1),
                            glm::vec4(vx1.x,vx1.y,0,1),
                            glm::u8vec4(0,255,255,255));

                poly_vx_buffer->insert(
                            poly_vx_buffer->end(),
                            line_buffer->begin(),
                            line_buffer->end());
            }

            auto polyvx_ent_id = m_scene->CreateEntity();

            m_cmlist_draw_data->Create(
                        polyvx_ent_id,
                        DrawData{
                            m_debug_draw_key,
                            std::move(poly_vx_buffer),
                            true
                        });

            TransformData xf_data;
            xf_data.world_xf = glm::mat4(1.0);

            m_cmlist_xf_data->Create(
                        polyvx_ent_id,
                        xf_data);

            m_list_debug_ent_ids.push_back(polyvx_ent_id);

            for(auto& child : widget->GetChildren())
            {
                stack_widgets.push_back(child.get());
            }
        }
    }

    void DrawSystem::createBoundingBoxDrawData()
    {
        // Get widget list
        std::vector<Widget*> stack_widgets;
        stack_widgets.push_back(m_scene->GetRootWidget().get());

        while(!stack_widgets.empty())
        {
            auto widget = stack_widgets.back();
            stack_widgets.pop_back();

            // Create bounding box entity and DrawData
            // for this widget
            auto const &bb =
                    m_cmlist_xf_data->
                    GetComponent(widget->GetEntityId()).bbox;

            auto line0_buffer =
                    raintk::draw_debug::CreateLine(
                        glm::vec4(bb.x0,bb.y0,0,1),
                        glm::vec4(bb.x0,bb.y1,0,1));

            auto line1_buffer =
                    raintk::draw_debug::CreateLine(
                        glm::vec4(bb.x1,bb.y0,0,1),
                        glm::vec4(bb.x1,bb.y1,0,1));

            auto line2_buffer =
                    raintk::draw_debug::CreateLine(
                        glm::vec4(bb.x0,bb.y0,0,1),
                        glm::vec4(bb.x1,bb.y0,0,1));

            auto line3_buffer =
                    raintk::draw_debug::CreateLine(
                        glm::vec4(bb.x0,bb.y1,0,1),
                        glm::vec4(bb.x1,bb.y1,0,1));

            UPtrBuffer bbox_buffer =
                    make_unique<std::vector<u8>>();

            bbox_buffer->insert(
                        bbox_buffer->end(),
                        line0_buffer->begin(),
                        line0_buffer->end());

            bbox_buffer->insert(
                        bbox_buffer->end(),
                        line1_buffer->begin(),
                        line1_buffer->end());

            bbox_buffer->insert(
                        bbox_buffer->end(),
                        line2_buffer->begin(),
                        line2_buffer->end());

            bbox_buffer->insert(
                        bbox_buffer->end(),
                        line3_buffer->begin(),
                        line3_buffer->end());

            auto bbox_ent_id = m_scene->CreateEntity();

            m_cmlist_draw_data->Create(
                        bbox_ent_id,
                        DrawData{
                            m_debug_draw_key,
                            std::move(bbox_buffer),
                            true
                        });

            TransformData xf_data;
            xf_data.world_xf = glm::mat4(1.0);

            m_cmlist_xf_data->Create(
                        bbox_ent_id,
                        xf_data);

            m_list_debug_ent_ids.push_back(bbox_ent_id);


            for(auto& child : widget->GetChildren())
            {
                stack_widgets.push_back(child.get());
            }
        }
    }

    void DrawSystem::setupDebugRendering()
    {
        m_debug_geometry_layout.reset(
                    new GeometryLayout{
                        ks::gl::VertexLayout
                        {
                            {
                                "a_v4_position",
                                raintk::draw_debug::AttrType::Float,
                                4,
                                false
                            },
                            {
                                "a_v4_color",
                                raintk::draw_debug::AttrType::UByte,
                                4,
                                true
                            }
                        },
                        sizeof(raintk::draw_debug::Vertex),
                        50000 // buffer size: 50000 bytes
                    });

        m_debug_shader_id =
                RegisterShader(
                    "bbox_shader",
                    raintk::draw_debug::color_attr_vert_glsl,
                    raintk::draw_debug::color_attr_frag_glsl);

        m_debug_depth_config_id =
                RegisterDepthConfig(
                    [](ks::gl::StateSet* state_set) {
                        state_set->SetDepthMask(GL_FALSE);
                        state_set->SetDepthTest(GL_FALSE);
                    });

        m_debug_blend_config_id =
                RegisterBlendConfig(
                    [](ks::gl::StateSet* state_set) {
                        state_set->SetBlend(GL_FALSE);
                    });

        m_debug_geometry_layout_id =
                RegisterGeometryLayout(
                    m_debug_geometry_layout);

        m_debug_draw_key.SetShader(m_debug_shader_id);
        m_debug_draw_key.SetDepthConfig(m_debug_depth_config_id);
        m_debug_draw_key.SetBlendConfig(m_debug_blend_config_id);
        m_debug_draw_key.SetPrimitive(ks::gl::Primitive::TriangleStrip);
        m_debug_draw_key.SetGeometryLayout(m_debug_geometry_layout_id);
        m_debug_draw_key.SetTransparency(false);
    }

    // ============================================================= //
}




