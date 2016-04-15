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

#include <raintk/RainTkInputArea.hpp>
#include <raintk/RainTkInputSystem.hpp>
#include <raintk/RainTkScene.hpp>
#include <raintk/RainTkLog.hpp>

namespace raintk
{
    InputArea::InputArea(ks::Object::Key const &key,
                         shared_ptr<Widget> parent,
                         std::string name) :
        Widget(key,parent,name),
        m_cmlist_input_data(
            static_cast<InputDataComponentList*>(
                m_scene->template GetComponentList<InputData>()))
    {

    }

    void InputArea::Init(ks::Object::Key const &,
                         shared_ptr<InputArea> const &this_area)
    {
        m_cid_enabled =
                enabled.signal_changed.Connect(
                    this_area,
                    &InputArea::onEnabledChanged,
                    ks::ConnectionType::Direct);

        // Create the Input Component
        auto& input_data =
                m_cmlist_input_data->
                Create(m_entity_id,InputData{});

        input_data.enabled = enabled.Get();
        input_data.input_area = this;
    }

    InputArea::~InputArea()
    {
        m_cmlist_input_data->Remove(m_entity_id);
    }

    void InputArea::cancelInputsBehindWidget(
            std::vector<Point> const &list_points)
    {
        // Convert list_points to world coords
        std::vector<Point> list_world_pts = list_points;
        for(auto& world_pt : list_world_pts)
        {
            glm::vec2 world_xy =
                    Widget::CalcWorldCoords(
                        this,
                        glm::vec2(world_pt.x,world_pt.y));

            world_pt.x = world_xy.x;
            world_pt.y = world_xy.y;
        }

        auto const & list_input_areas_by_depth =
                m_scene->GetInputSystem()->
                GetInputAreasByDepth();

        auto const this_depth =
                m_cmlist_xf_data->GetComponent(
                    m_entity_id).world_xf[3].z;

        for(auto const &depth_ipa : list_input_areas_by_depth)
        {
            if(this_depth > depth_ipa.first)
            {
                for(auto const &cancel_pt : list_world_pts)
                {
                    // cancel_pt is in world coordinates
                    bool const inside =
                            Widget::CalcPointInside(
                                depth_ipa.second,
                                glm::vec2(
                                    cancel_pt.x,
                                    cancel_pt.y));

                    if(inside)
                    {
                        depth_ipa.second->cancelInput();
                        break;
                    }
                }
            }
        }
    }

    void InputArea::onEnabledChanged()
    {
        m_cmlist_input_data->GetComponent(m_entity_id).enabled = enabled.Get();
    }
}

