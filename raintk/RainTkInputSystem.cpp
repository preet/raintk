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

#include <ks/gui/KsGuiApplication.hpp>

#include <raintk/RainTkInputSystem.hpp>
#include <raintk/RainTkScene.hpp>
#include <raintk/RainTkInputRecorder.hpp>
#include <raintk/RainTkInputListener.hpp>
#include <raintk/RainTkAnimation.hpp>
#include <raintk/RainTkLog.hpp>

namespace raintk
{
    // ============================================================= //
    // ============================================================= //

    using InputType = InputArea::Point::Type;

    class InputReplay : public raintk::Animation
    {
    public:
        using base_type = raintk::Animation;

        InputReplay(ks::Object::Key const &key,
                    Scene* scene,
                    std::string const &input_file_path) :
            raintk::Animation(key,scene)
        {
            std::ifstream input_file(input_file_path);

            // Parse input file
            std::string line;
            while(std::getline(input_file,line))
            {
                std::string token;
                std::istringstream iss(line);

                // frame
                std::getline(iss,token,',');
                uint frame = StringToUInt(token);

                // frame time
                std::getline(iss,token,',');
                m_frame_times_by_frame.emplace(
                            frame,
                            TimePoint(Milliseconds(StringToUInt(token))));

                InputArea::Point p;

                // timestamp
                std::getline(iss,token,',');
                p.timestamp = TimePoint(Milliseconds(StringToUInt(token)));

                // type
                token.clear();
                std::getline(iss,token,',');
                p.type = static_cast<InputArea::Point::Type>(StringToUInt(token));

                // button
                token.clear();
                std::getline(iss,token,',');
                p.button = static_cast<InputArea::Point::Button>(StringToUInt(token));

                // action
                token.clear();
                std::getline(iss,token,',');
                p.action = static_cast<InputArea::Point::Action>(StringToUInt(token));

                // x
                token.clear();
                std::getline(iss,token,',');
                p.x = StringToFloat(token);

                // y
                token.clear();
                std::getline(iss,token,',');
                p.y = StringToFloat(token);

                m_inputs_by_frame.emplace(frame,p);
            }

            // Fill in frame times
            uint start_frame = m_inputs_by_frame.begin()->first;
            uint last_frame = m_inputs_by_frame.rbegin()->first;

            TimePoint prev_time = m_frame_times_by_frame.begin()->second;

            for(uint f=start_frame+1; f <= last_frame; f++)
            {
                if(m_frame_times_by_frame.count(f) == 0)
                {
                    m_frame_times_by_frame.emplace(f,prev_time+Milliseconds(16));
                }
                else
                {
                    prev_time = m_frame_times_by_frame[f];
                }
            }

            m_start_frame = m_inputs_by_frame.begin()->first;
            m_last_frame = m_inputs_by_frame.rbegin()->first;
        }

        void Init(ks::Object::Key const &,
                  shared_ptr<InputReplay> const &)
        {}

        ~InputReplay()
        {}

        void start() override
        {
            m_frame = m_start_frame;
        }

        bool update(float) override
        {
            m_list_points.clear();

            auto it_range = m_inputs_by_frame.equal_range(m_frame);
            for(auto it = it_range.first; it != it_range.second; ++it)
            {
                m_list_points.push_back(it->second);
            }

            m_frame++;
            if(m_frame > m_last_frame)
            {
                return true;
            }

            return false;
        }

        void complete() override
        {}

        std::vector<InputArea::Point> GetPoints()
        {
            return m_list_points;
        }

        static u64 StringToUInt(std::string const &s)
        {
            u64 i=0;
            std::stringstream ss(s);
            ss >> i;
            return i;
        }

        static float StringToFloat(std::string const &s)
        {
            float f = 0;
            std::stringstream ss(s);
            ss >> f;
            return f;
        }

    private:
        std::multimap<uint,InputArea::Point> m_inputs_by_frame;
        std::map<uint,TimePoint> m_frame_times_by_frame;

        uint m_frame{0};
        uint m_start_frame;
        uint m_last_frame;
        std::vector<InputArea::Point> m_list_points;
    };

    // ============================================================= //
    // ============================================================= //

    InputSystem::InputSystem(Scene* scene,
                             ks::gui::Application* app) :
        m_scene(scene),
        m_app(app)
    {
        // Get the InputData mask
        m_inputable_mask =
                m_scene->template GetComponentMask<InputData,TransformData>();

        // Create the InputData component list
        m_scene->template RegisterComponentList<InputData>(
                    make_unique<InputDataComponentList>(*m_scene));

        m_cmlist_input_data =
                static_cast<InputDataComponentList*>(
                    m_scene->template GetComponentList<InputData>());

        // Create the InputListener
        m_input_listener = ks::MakeObject<InputListener>(app);
    }

    InputSystem::~InputSystem()
    {

    }

    std::string InputSystem::GetDesc() const
    {
        return "raintk InputSystem";
    }

    void InputSystem::Update(TimePoint const &prev_upd_time,
                             TimePoint const &curr_upd_time)
    {
        auto& list_entities = m_scene->GetEntityList();
        auto& list_input_data = m_cmlist_input_data->GetSparseList();

        auto const & list_xf_data =
                static_cast<TransformDataComponentList*>(
                    m_scene->template GetComponentList<TransformData>())->
                        GetSparseList();

        m_list_input_areas_by_depth.clear();

        for(uint ent_id=0; ent_id < list_entities.size(); ent_id++)
        {
            if((list_entities[ent_id].mask & m_inputable_mask)==m_inputable_mask)
            {
                auto& input_data = list_input_data[ent_id];
                auto const &xf_data = list_xf_data[ent_id];

                // NOTE: We need to check if the transform data is valid.
                // TransformData is invalid until the TransformSystem has
                // updated it for the first time.
                if(input_data.enabled && xf_data.valid)
                {
                    m_list_input_areas_by_depth.emplace_back(
                                xf_data.world_xf[3].z,
                                input_data.input_area);
                }
            }
        }

        // Sort the InputAreas by depth
        std::sort(
                    m_list_input_areas_by_depth.begin(),
                    m_list_input_areas_by_depth.end(),
                    [](std::pair<float,InputArea*> const &a,
                       std::pair<float,InputArea*> const &b) {
                        return(a.first > b.first);
                    });

        if(m_input_recorder)
        {
            m_input_recorder->Update(curr_upd_time);
        }

        // Get Input points
        auto list_points =
                m_input_listener->GetInputs(
                    prev_upd_time,curr_upd_time);

        if(m_input_replay)
        {
            // Overwrite points with replay data
            list_points = m_input_replay->GetPoints();
        }

        while(list_points.empty()==false)
        {
            for(uint i=0; i < m_list_input_areas_by_depth.size();)
            {
                auto input_area = m_list_input_areas_by_depth[i].second;
                auto const &world_pt = list_points[0];

                glm::vec2 world_xy(world_pt.x,world_pt.y);
                glm::vec2 local_xy = Widget::CalcLocalCoords(input_area,world_xy);
                bool inside = Widget::CalcPointInside(input_area,world_xy,local_xy);

                auto local_pt = world_pt;
                local_pt.x = local_xy.x;
                local_pt.y = local_xy.y;

                auto const response = input_area->handleInput(local_pt,inside);

                if(response == InputArea::Response::Accept)
                {
                    list_points.erase(list_points.begin());

                    if(list_points.empty())
                    {
                        break;
                    }
                    else
                    {
                        // Start back at the beginning
                        i=0;
                    }
                }
                else
                {
                    // Try the current point with
                    // the next InputArea
                    i++;
                }
            }

            if(list_points.empty())
            {
                break;
            }

            // When we get here the first entry in list_points
            // has been rejected by all InputAreas. Discard it
            // and try from the beginning with the next point.
            list_points.erase(list_points.begin());
        }
    }

    InputDataComponentList*
    InputSystem::GetInputDataComponentList() const
    {
        return m_cmlist_input_data;
    }

    std::vector<std::pair<float,InputArea*>> const &
    InputSystem::GetInputAreasByDepth() const
    {
        return m_list_input_areas_by_depth;
    }

    void InputSystem::SetInputFocus(shared_ptr<Widget> const &focus_widget)
    {
        m_input_listener->SetInputFocus(focus_widget);
    }

    void InputSystem::ClearInputFocus()
    {
        m_input_listener->ClearInputFocus();
    }

    void InputSystem::StartInputRecording(std::string const &file_name)
    {
        m_input_recorder = nullptr;

        m_input_recorder =
                ks::MakeObject<InputRecorder>(
                    m_app,file_name,18000); // 5 min
    }

    void InputSystem::StopInputRecording()
    {
        m_input_recorder = nullptr;
    }

    void InputSystem::StartInputPlayback(std::string const &file_name)
    {
        m_input_replay = nullptr;

        m_input_replay =
                ks::MakeObject<InputReplay>(
                    m_scene,file_name);

        m_input_replay->Start();
    }

    void InputSystem::StopInputPlayback()
    {
        m_input_replay = nullptr;
    }

    // ============================================================= //
    // ============================================================= //
}
