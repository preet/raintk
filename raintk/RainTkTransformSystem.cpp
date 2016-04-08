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

#include <raintk/RainTkAnimationSystem.hpp>
#include <raintk/RainTkTransformSystem.hpp>
#include <raintk/RainTkScene.hpp>
#include <raintk/RainTkWidget.hpp>
#include <raintk/RainTkLog.hpp>
#include <raintk/RainTkAnimation.hpp>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/string_cast.hpp>

namespace raintk
{
    TransformSystem::TransformSystem(Scene* scene) :
        m_scene(scene)
    {
        // Create the UpdateData component list
        m_scene->template RegisterComponentList<UpdateData>(
                    make_unique<UpdateDataComponentList>(*m_scene));

        m_cmlist_upd_data =
                static_cast<UpdateDataComponentList*>(
                    m_scene->template GetComponentList<UpdateData>());

        // Create the TransformData component list
        m_scene->template RegisterComponentList<TransformData>(
                    make_unique<TransformDataComponentList>(*m_scene));

        m_cmlist_xf_data =
                static_cast<TransformDataComponentList*>(
                    m_scene->template GetComponentList<TransformData>());
    }

    TransformSystem::~TransformSystem()
    {

    }

    std::string TransformSystem::GetDesc() const
    {
        return "raintk TransformSystem";
    }

    TransformDataComponentList*
    TransformSystem::GetTransformDataComponentList() const
    {
        return m_cmlist_xf_data;
    }

    void TransformSystem::Update(TimePoint const &/*prev_time*/,
                                 TimePoint const &/*curr_time*/)
    {
        updateLayout();
        updateTransforms();
        updateAnimations(); // must be after updateLayout
    }

    void TransformSystem::updateLayout()
    {
        // Handle requests for general widget updates in
        // multiple passes until all updates are satisfied
        // or we've reached some limit

        auto& list_upd_data = m_cmlist_upd_data->GetSparseList();

        uint const max_upd_loop_count=3;

        for(uint i=0; i < max_upd_loop_count; i++)
        {
            bool updated_widgets=false;

            // Ensure all queued signals are handled as they may
            // cause the UpdateData state of entities to change

            // Currently instead of doing this, we use a direct
            // connection for all property change notificiations
            // m_scene->GetEventLoop()->ProcessEvents();

            for(uint ent_id=0; ent_id < list_upd_data.size(); ent_id++)
            {
                auto& upd_data = list_upd_data[ent_id];

                if(upd_data.update & UpdateData::UpdateWidget)
                {
                    upd_data.widget->update();
                    upd_data.update &= ~(UpdateData::UpdateWidget);
                    updated_widgets = true;
                }
            }

            if(!updated_widgets)
            {
                break;
            }

            if(i == max_upd_loop_count-1)
            {
                // TODO throw?
                rtklog.Warn() << "UpdateSystem: "
                              << "Too many update loops with "
                                 "updated widgets still remaining";
            }
        }
    }

    void TransformSystem::updateTransforms()
    {
        // Update the Transform hierarchy using the Widget
        // parent/child tree
        updateWidgetTransforms(
                    m_scene->GetRootWidget().get(),
                    glm::mat4{1.0},
                    m_cmlist_upd_data,
                    m_cmlist_xf_data);
    }

    void TransformSystem::updateAnimations()
    {
        // TransformSystem::updateLayout must be completed in
        // between Animation::Start() and Animation::Update()
        // to ensure that up to date values are used for the
        // animation

        // If Animation::Start() is called during updateLayout,
        // nothing needs to be done. However if it is called
        // before or after updateLayout(), Animation::Update()
        // must be delayed until updateLayout() is called

        // To ensure this happens, we set the Animation's
        // m_ready flag to false in Animation::Start(), and
        // set it to true once updateLayout has completed

        // The AnimationSystem (which runs the following frame,
        // after TransformSystem) will only update Animations
        // that have m_ready set to true

        auto const &list_animations =
                m_scene->GetAnimationSystem()->
                GetListAnimations();

        for(Animation* animation : list_animations)
        {
            animation->m_ready = true;
        }
    }

    // TODO make iterative
    void TransformSystem::updateWidgetTransforms(
            Widget* widget,
            glm::mat4 const &parent_xf,
            UpdateDataComponentList* cmlist_upd_data,
            TransformDataComponentList* cmlist_xf_data)
    {
        auto const ent_id = widget->GetEntityId();
        auto& xf_data = cmlist_xf_data->GetComponent(ent_id);
        auto& update_data = cmlist_upd_data->GetComponent(ent_id);

        if(update_data.update & UpdateData::UpdateTransform)
        {
            // Recalculate the world transform
            glm::vec3 const xf_data_origin(xf_data.origin,0.0f);

            xf_data.world_xf =
                    parent_xf *
                    glm::translate(xf_data.position) *
                    glm::translate(xf_data_origin) *
                    glm::rotate(xf_data.rotation,glm::vec3(0.0f,0.0f,1.0f)) *
                    glm::scale(glm::vec3(xf_data.scale,1.0f)) *
                    glm::translate(xf_data_origin*-1.0f);

            widget->onTransformUpdated();
            update_data.update &= ~(UpdateData::UpdateTransform);
            xf_data.valid = true;

            // Update the bounding box
            auto const width = widget->width.Get();
            auto const height = widget->height.Get();

            glm::vec4 obb[4];
            obb[0] = xf_data.world_xf*glm::vec4(0.0f,0.0f,0.0f,1.0f);
            obb[1] = xf_data.world_xf*glm::vec4(width,0.0f,0.0f,1.0f);
            obb[2] = xf_data.world_xf*glm::vec4(0.0f,height,0.0f,1.0f);
            obb[3] = xf_data.world_xf*glm::vec4(width,height,0.0f,1.0f);

            xf_data.bbox.x0 = obb[0].x;
            xf_data.bbox.x1 = obb[0].x;
            xf_data.bbox.y0 = obb[0].y;
            xf_data.bbox.y1 = obb[0].y;

            for(uint i=1; i < 4; i++)
            {
                xf_data.bbox.x0 = std::min(xf_data.bbox.x0, obb[i].x);
                xf_data.bbox.x1 = std::max(xf_data.bbox.x1, obb[i].x);
                xf_data.bbox.y0 = std::min(xf_data.bbox.y0, obb[i].y);
                xf_data.bbox.y1 = std::max(xf_data.bbox.y1, obb[i].y);
            }


            // Mark all child transforms as requiring updates
            for(auto& child : widget->GetChildren())
            {
                auto& upd_data =
                        cmlist_upd_data->
                        GetComponent(child->GetEntityId());

                upd_data.update |= UpdateData::UpdateTransform;
            }
        }

        for(auto &child : widget->GetChildren())
        {
            updateWidgetTransforms(child.get(),
                                   xf_data.world_xf,
                                   cmlist_upd_data,
                                   cmlist_xf_data);
        }
    }    
}

