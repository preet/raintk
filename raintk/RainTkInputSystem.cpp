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
#include <raintk/RainTkInputListener.hpp>
#include <raintk/RainTkLog.hpp>

namespace raintk
{
    // ============================================================= //
    // ============================================================= //

    using InputType = InputArea::Point::Type;

    namespace
    {
        InputArea::Point ConvertToInputPoint(ks::gui::MouseEvent const &mouse_event)
        {
            InputArea::Point mouse_point {
                InputArea::Point::Type::Mouse,
                static_cast<InputArea::Point::Button>(mouse_event.button),
                static_cast<InputArea::Point::Action>(mouse_event.action),
                px(mouse_event.x),
                px(mouse_event.y),
                mouse_event.timestamp
            };

            return mouse_point;
        }

        InputArea::Point ConvertToInputPoint(ks::gui::TouchEvent const &touch_event)
        {
            InputArea::Point touch_point{
                // Type=0 is Mouse, Type=1 is Touch0, etc
                static_cast<InputArea::Point::Type>(touch_event.index+1),
                InputArea::Point::Button::None,
                static_cast<InputArea::Point::Action>(touch_event.action),
                px(touch_event.x),
                px(touch_event.y),
                touch_event.timestamp
            };

            return touch_point;
        }
    }

    // ============================================================= //
    // ============================================================= //

    class InputRecorder : public ks::Object
    {
    public:
        using base_type = ks::Object;

        InputRecorder(ks::Object::Key const &key,
                      ks::gui::Application* app,
                      std::string const &file_path) :
            ks::Object(key,app->GetEventLoop()),
            m_app(app),
            m_file(file_path,std::ios_base::out | std::ios_base::trunc)
        {}

        void Init(ks::Object::Key const &,
                  shared_ptr<InputRecorder> const &this_recorder)
        {
            // Setup connections
            m_cid_mouse_events =
                    m_app->signal_mouse_input->Connect(
                        this_recorder,
                        &InputRecorder::onMouseEvent,
                        ks::ConnectionType::Direct);

            m_cid_touch_events =
                    m_app->signal_touch_input->Connect(
                        this_recorder,
                        &InputRecorder::onTouchEvent,
                        ks::ConnectionType::Direct);
        }

        ~InputRecorder()
        {}

        void Update(TimePoint const &curr_upd_time)
        {
            m_frame++;
            m_frame_time = curr_upd_time;
        }

    private:
        void onMouseEvent(ks::gui::MouseEvent mouse_event)
        {
            recordInput(ConvertToInputPoint(mouse_event));
        }

        void onTouchEvent(ks::gui::TouchEvent touch_event)
        {
            // We don't currently support more than 3 touch inputs
            if(touch_event.index > 2)
            {
                return;
            }

            recordInput(ConvertToInputPoint(touch_event));
        }

        void recordInput(InputArea::Point const &point)
        {
            // Format is
            // frame,frame_time,timestamp,type,button,action,x,y\n

            std::string line;
            line += ks::ToString(m_frame) + ",";
            line += ks::ToString(
                        static_cast<u64>(
                            std::chrono::duration_cast<Milliseconds>(
                                m_frame_time.time_since_epoch()).count())) + ",";
            line += ks::ToString(
                        static_cast<u64>(
                            std::chrono::duration_cast<Milliseconds>(
                                point.timestamp.time_since_epoch()).count())) + ",";
            line += ks::ToString(static_cast<uint>(point.type)) + ",";
            line += ks::ToString(static_cast<uint>(point.button)) + ",";
            line += ks::ToString(static_cast<uint>(point.action)) + ",";
            line += ks::ToString(static_cast<float>(point.x)) + ",";
            line += ks::ToString(static_cast<float>(point.y)) + "\n";

            rtklog.Trace() << line;
            m_file << line;
        }

        ks::gui::Application* m_app;
        uint m_frame{0};
        TimePoint m_frame_time;
        std::ofstream m_file;
        Id m_cid_mouse_events{0};
        Id m_cid_touch_events{0};
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

    void InputSystem::StartInputRecording(std::string const &file_name)
    {
        m_input_recorder = nullptr;

        m_input_recorder =
                ks::MakeObject<InputRecorder>(
                    m_app,file_name);
    }

    void InputSystem::StopInputRecording()
    {
        m_input_recorder = nullptr;
    }
}
