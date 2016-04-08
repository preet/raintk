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

    InputArea::Point InputArea::TransformPtToLocalCoords(Point const &point) const
    {
        auto xf_point = point;

        // Transform the point so its in local coords
        auto const &xf = m_cmlist_xf_data->
                GetComponent(m_entity_id).world_xf;

        glm::vec4 const local =
                glm::inverse(xf)*
                glm::vec4(xf_point.x,xf_point.y,0,1);

        xf_point.x = local.x;
        xf_point.y = local.y;

        return xf_point;
    }

    void InputArea::cancelInputsBehindDepth(
            InputSystem* input_system,
            std::vector<Point> const &list_points,
            float depth)
    {
        auto const & list_input_areas_by_depth =
                input_system->GetInputAreasByDepth();

        for(auto const &depth_ipa : list_input_areas_by_depth)
        {
            if(depth > depth_ipa.first)
            {
                depth_ipa.second->cancelInput(list_points);
            }
        }
    }

    void InputArea::onEnabledChanged()
    {
        m_cmlist_input_data->GetComponent(m_entity_id).enabled = enabled.Get();
    }
}

