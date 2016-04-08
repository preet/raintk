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

#include <ks/gui/KsGuiApplication.hpp>

#include <raintk/RainTkInputSystem.hpp>
#include <raintk/RainTkScene.hpp>
#include <raintk/RainTkInputArea.hpp>
#include <raintk/RainTkLog.hpp>

namespace raintk
{
    // ============================================================= //
    // ============================================================= //

    using InputType = InputArea::Point::Type;

    class InputListener : public ks::Object
    {
    public:
        using base_type = ks::Object;

        InputListener(ks::Object::Key const &key,
                      shared_ptr<ks::EventLoop> event_loop) :
            ks::Object(key,event_loop)
        {
            std::pair<bool,InputArea::Point> pressed_point;
            pressed_point.first = false;

//            list_lk_pressed_by_type.resize(
//                        uint(InputArea::Point::Type::TypeCount),
//                        pressed_point);
        }

        void Init(ks::Object::Key const &,
                  shared_ptr<InputListener> const &)
        {}

        ~InputListener()
        {}

        void OnMouseInput(ks::gui::MouseEvent mouse_event)
        {
//            uint const mouse_type_idx = uint(InputArea::Point::Type::Mouse);

            InputArea::Point mouse_point {
                InputArea::Point::Type::Mouse,
                static_cast<InputArea::Point::Button>(mouse_event.button),
                static_cast<InputArea::Point::Action>(mouse_event.action),
                px(mouse_event.x),
                px(mouse_event.y),
                mouse_event.timestamp
            };

            list_points.push_back(mouse_point);

            // NOTE: We no longer do this, see InputSystem
            // comments for more details
//            // Save the most recent mouse_event if any mouse
//            // buttons are being pressed so that the InputSystem
//            // can repeat last pressed events
//            if(mouse_point.action == InputArea::Point::Action::Press)
//            {
//                list_lk_pressed_by_type[mouse_type_idx].first = true;
//            }
//            else if(mouse_point.action == InputArea::Point::Action::Release)
//            {
//                list_lk_pressed_by_type[mouse_type_idx].first = false;
//            }

//            if(list_lk_pressed_by_type[mouse_type_idx].first)
//            {
//                list_lk_pressed_by_type[mouse_type_idx].second = mouse_point;
//            }
        }

        void ResampleInputs(TimePoint const &prev_upd_time,
                            TimePoint const &curr_upd_time)
        {
            // For some inputs (mouse) on SDL it seems like
            // the timestamp reflects when the event is polled
            // rather than when it actually occurs

            // So we just assign the timestamp to always mean
            // when its polled

            // This means the timestamp of the event represents
            // *the end of the interval the event occured in*

            // To convert the timestamps to reflect when the
            // event occured, we resample by linearly interpolating
            // the events over the interval (this is a guess)

            // @prev_upd_time: start of the interval
            // @curr_upd_time: end of the interval

            // We assume that list_points is cleared between
            // each call to ResampleInputs

            if(list_points.empty())
            {
                return;
            }

            uint const point_count = list_points.size();

            Microseconds avg_interval =
                    ks::CalcDuration<Microseconds>(
                        prev_upd_time,
                        curr_upd_time);

            avg_interval = Microseconds(avg_interval.count()/(point_count+1));

            for(uint i=0; i < point_count; i++)
            {
                list_points[i].timestamp =
                        prev_upd_time +
                        Microseconds(avg_interval.count()*(i+1));
            }
        }


        std::vector<InputArea::Point> list_points;

//        std::vector<
//            std::pair<bool,InputArea::Point>
//        > list_lk_pressed_by_type;
    };

    // ============================================================= //
    // ============================================================= //

    InputSystem::InputSystem(Scene* scene,
                             ks::gui::Application* app) :
        m_scene(scene)
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
        m_input_listener =
                ks::MakeObject<InputListener>(
                    scene->GetEventLoop());

        app->signal_mouse_input->Connect(
                    m_input_listener,
                    &InputListener::OnMouseInput,
                    ks::ConnectionType::Direct);
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

        // Sort the InputAreas by height // TODO rename height --> depth!
        std::sort(
                    m_list_input_areas_by_depth.begin(),
                    m_list_input_areas_by_depth.end(),
                    [](std::pair<float,InputArea*> const &a,
                       std::pair<float,InputArea*> const &b) {
                        return(a.first > b.first);
                    });

        // Resample input times
        m_input_listener->ResampleInputs(
                    prev_upd_time,curr_upd_time);

        auto& list_points = m_input_listener->list_points;

        // NOTE (start)
        // We initially repeated inputs that were previously
        // pressed but hadn't been released to be able to
        // detect input speed continuously. This is currently
        // disabled and current widgets will probably break
        // if its reenabled (ie ScrollArea)

//        // If list_points is empty, fill it with any inputs
//        // that were previously pressed but haven't yet been
//        // released

//        if(list_points.empty())
//        {
//            for(auto& pressed_point : m_input_listener->list_lk_pressed_by_type)
//            {
//                if(pressed_point.first)
//                {
//                    list_points.push_back(pressed_point.second);
//                    list_points.back().timestamp = curr_upd_time;
//                }
//            }
//        }

        // NOTE (end)

        while(list_points.empty()==false)
        {
            for(uint i=0; i < m_list_input_areas_by_depth.size();)
            {
                auto const response =
                        m_list_input_areas_by_depth[i].second->handleInput(
                            list_points[0]);

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
}
