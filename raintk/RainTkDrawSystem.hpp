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

#ifndef RAINTK_DRAW_SYSTEM_HPP
#define RAINTK_DRAW_SYSTEM_HPP

#include <ks/draw/KsDrawSystem.hpp>
#include <ks/draw/KsDrawRenderSystem.hpp>
#include <ks/shared/KsRecycleIndexList.hpp>
#include <raintk/RainTkGlobal.hpp>
#include <raintk/RainTkComponents.hpp>

namespace raintk
{
    class Scene;

    class DrawSystem : public ks::draw::System
    {
    public:
        DrawSystem(Scene* scene, RenderSystem* render_system);
        ~DrawSystem();

        std::string GetDesc() const override;
        std::vector<BoundingBox> const &GetClipRegions() const;

        std::vector<Id> GetOpaqueDrawOrderList() const;
        std::vector<Id> GetTransparentDrawOrderList() const;

        DrawDataComponentList*
        GetDrawDataComponentList() const;

        void SetClippingEnabled(bool enabled);
        void SetShowBoundingBoxes(bool show_bboxes);

        Id RegisterGeometryLayout(shared_ptr<GeometryLayout const> gm_layout);
        void RemoveGeometryLayout(Id gm_layout_id);

        // Wrap some ks::draw::RenderSystem functions since we
        // don't want to expose RenderSystem in the raintk API
        Id RegisterShader(std::string shader_desc,
                          std::string shader_source_vsh,
                          std::string shader_source_fsh);

        void RemoveShader(Id shader_id);

        Id RegisterDepthConfig(ks::draw::StateSetCb depth_config);
        void RemoveDepthConfig(Id depth_config_id);

        Id RegisterBlendConfig(ks::draw::StateSetCb blend_config);
        void RemoveBlendConfig(Id blend_config_id);

        Id RegisterStencilConfig(ks::draw::StateSetCb stencil_config);
        void RemoveStencilConfig(Id stencil_config_id);

        Id RegisterTextureSet(shared_ptr<ks::draw::TextureSet> texture_set);
        void RemoveTextureSet(Id texture_set_id);

        Id RegisterUniformSet(shared_ptr<ks::draw::UniformSet> uniform_set);
        void RemoveUniformSet(Id uniform_set_id);

        void Reset(); // TODO

        void Update(TimePoint const &prev_time,
                    TimePoint const &curr_time) override;


#ifndef RAINTK_DEBUG_TEST_DRAW_SYSTEM
    private:
#endif
        void sortIntoOpaqueGroups(
                std::vector<Id> &list_opq_draw_data);

        void sortIntoTransparencyGroups(
                std::vector<Id> &list_xpr_draw_data);

        void createRenderDataForCommonKeyGroups(
                ks::draw::Transparency transparency,
                std::vector<std::vector<DrawData*>> &list_common_key_groups,
                std::vector<Id>& list_render_ent_ids);

        void createRenderDataForDrawData(
                DrawKey const &key,
                ks::draw::BufferLayout const * buffer_layout,
                std::vector<DrawData*> const &list_draw_data,
                ks::draw::Transparency transparency,
                std::vector<Id>& list_render_ent_ids);

        void createBoundingBoxDrawData();

        void setupBoundingBoxRendering();


        Scene* const m_scene;
        RenderSystem* const m_render_system;
        UpdateDataComponentList* const m_cmlist_upd_data;
        TransformDataComponentList* const m_cmlist_xf_data;

        DrawDataComponentList* m_cmlist_draw_data;

        bool m_show_bboxes{false};
        Id m_bbox_shader_id{0};
        Id m_bbox_depth_config_id{0};
        Id m_bbox_blend_config_id{0};
        Id m_bbox_geometry_layout_id{0};
        DrawKey m_bbox_draw_key;
        shared_ptr<GeometryLayout> m_bbox_geometry_layout;
        std::vector<Id> m_list_bbox_ent_ids;

        ks::RecycleIndexList<
            shared_ptr<GeometryLayout const>
        > m_list_gm_layouts;

        // <GeometryLayout Id, BufferLayout>
        std::map<
            Id,
            unique_ptr<ks::draw::BufferLayout>
        > m_lkup_gm_buffer_layouts;

        bool m_clipping_enabled{true};

        // The list of clip regions where indices correspond to
        // DrawKey clip id
        std::vector<BoundingBox> m_list_clip_regions;

        // RenderData entities sorted in the correct draw order
        std::vector<Id> m_list_opq_render_ent_ids;
        std::vector<Id> m_list_xpr_render_ent_ids;
    };
}

#endif // RAINTK_DRAW_SYSTEM_HPP



