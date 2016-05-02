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

//        std::string DebugTimePointToString(TimePoint const &time_point)
//        {
//            return "";

//            return ks::ToString(static_cast<u64>(
//                                  std::chrono::duration_cast<Milliseconds>(
//                                      time_point.time_since_epoch()).count()));
//        }
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

//    class InputListener : public ks::Object
//    {
//        struct InputFrame
//        {
//            TimePoint t0{Milliseconds(0)};
//            TimePoint t1{Milliseconds(0)};
//        };

//        struct InputPoint
//        {
//            uint frame;
//            InputArea::Point point;
//        };

//        ks::gui::Application* m_app;

//        uint m_input_buffer_size{4}; // input buffer size in frames
//        uint m_input_buffer_delay{2}; // input buffer delay in frames,
//                                      // must be less than m_input_buffer_size
//        std::vector<std::vector<InputFrame>> m_lkup_input_frames;
////        std::vector<std::vector<InputArea::Point>> m_lkup_input_history;

//        std::vector<std::vector<InputPoint>> m_lkup_input_history; // use this

//    public:
//        using base_type = ks::Object;

//        InputListener(ks::Object::Key const &key,
//                      ks::gui::Application* app) :
//            ks::Object(key,app->GetEventLoop()),
//            m_app(app),
//            m_lkup_input_frames(static_cast<uint>(InputType::TypeCount)),
//            m_lkup_input_history(static_cast<uint>(InputType::TypeCount))
//        {
//            m_lkup_input_frames[static_cast<uint>(InputType::Mouse)].resize(m_input_buffer_size);
//            m_lkup_input_frames[static_cast<uint>(InputType::Touch0)].resize(m_input_buffer_size);
//            m_lkup_input_frames[static_cast<uint>(InputType::Touch1)].resize(m_input_buffer_size);
//            m_lkup_input_frames[static_cast<uint>(InputType::Touch2)].resize(m_input_buffer_size);
//        }

//        void Init(ks::Object::Key const &,
//                  shared_ptr<InputListener> const &this_listener)
//        {
//            m_app->signal_mouse_input->Connect(
//                        this_listener,
//                        &InputListener::OnMouseEvent,
//                        ks::ConnectionType::Direct);

//            m_app->signal_touch_input->Connect(
//                        this_listener,
//                        &InputListener::OnTouchEvent,
//                        ks::ConnectionType::Direct);

//            m_app->signal_resume.Connect(
//                        this_listener,
//                        &InputListener::OnAppResume,
//                        ks::ConnectionType::Direct);
//        }

//        ~InputListener()
//        {}

//        // Should be called every frame
//        std::vector<InputArea::Point>
//        GetInputs(TimePoint const &prev_upd_time,
//                  TimePoint const &curr_upd_time)
//        {
//            // All input points for the delayed frame
//            std::vector<InputArea::Point> list_all_points;

//            uint input_type_idx=0;
//            for(auto& list_input_frames : m_lkup_input_frames)
//            {
//                InputType input_type = static_cast<InputType>(input_type_idx);
//                input_type_idx++;

//                auto& oldest_frame = list_input_frames[0];

//                // Trim input history
//                trimHistoryBefore(input_type,oldest_frame.t0); // use t1 instead?

//                // Trim input buffer
//                list_input_frames.erase(list_input_frames.begin());

//                // Add new input frame
//                list_input_frames.emplace_back();
//                auto& newest_frame = list_input_frames.back();
//                newest_frame.t0 = prev_upd_time;
//                newest_frame.t1 = curr_upd_time;

//                auto& target_frame = list_input_frames[
//                        m_input_buffer_size-m_input_buffer_delay];

//                auto list_points =
//                        collectPointsInRange(
//                            input_type,
//                            target_frame.t0,
//                            target_frame.t1);

//                auto target_time = target_frame.t1 - Milliseconds(1);

//                InputArea::Point interp_point;
//                if(sampleInputHistory(input_type,target_time,interp_point))
//                {
//                    if(list_points.empty())
//                    {
//                        list_points.push_back(interp_point);
//                    }
//                    else
//                    {
//                        for(auto &pt : list_points)
//                        {
//                            pt.x = interp_point.x;
//                            pt.y = interp_point.y;
//                        }
//                    }
//                }

//                // Save the points
//                list_all_points.insert(
//                            list_all_points.end(),
//                            list_points.begin(),
//                            list_points.end());
//            }

//            return list_all_points;
//        }



//        // [t0, t1)
//        std::vector<InputArea::Point>
//        collectPointsInRange(InputType input_type,
//                             TimePoint const &t0,
//                             TimePoint const &t1)
//        {
//            auto& input_history = m_lkup_input_history[uint(input_type)];

//            std::vector<InputArea::Point> list_points;
//            for(auto const &pt : input_history)
//            {
//                if(pt.timestamp < t0 || pt.timestamp >= t1)
//                {
//                    continue;
//                }

//                list_points.push_back(pt);
//            }

//            return list_points;
//        }

//        void trimHistoryBefore(InputType input_type,
//                               TimePoint const &time)
//        {
//            auto& input_history = m_lkup_input_history[uint(input_type)];

//            input_history.erase(
//                        std::remove_if(
//                            input_history.begin(),
//                            input_history.end(),
//                            [&](InputArea::Point const &p){
//                                return p.timestamp < time;
//                            }),
//                        input_history.end());
//        }

//        bool sampleInputHistory(InputType input_type,
//                                TimePoint const &target_time,
//                                InputArea::Point &point)
//        {
//            auto& input_history = m_lkup_input_history[uint(input_type)];

//            point.timestamp = target_time;

//            auto greater_than_or_equal_it =
//                    std::lower_bound(
//                        input_history.begin(),
//                        input_history.end(),
//                        point,
//                        [&](InputArea::Point const &a, InputArea::Point const &b){
//                            return (a.timestamp < b.timestamp);
//                        });

//            if(greater_than_or_equal_it == input_history.end())
//            {
//                // No input points that occur after the target time
//                return false;
//            }

//            auto& greater_than_or_equal_pt = *greater_than_or_equal_it;

//            if(greater_than_or_equal_pt.timestamp == point.timestamp)
//            {
//                // There's already an Input point with the same timestamp
//                point = greater_than_or_equal_pt;
//                return true;
//            }
//            if(greater_than_or_equal_it == input_history.begin())
//            {
//                // The target time occurs before the earliest
//                // known input history
//                return false;
//            }

//            auto less_than_it = greater_than_or_equal_it;
//            std::advance(less_than_it,-1);

//            auto& less_than_pt = *less_than_it;

//            if(less_than_pt.action == InputArea::Point::Action::Release)
//            {
//                // We can't interpolate as the input has ended
//                return false;
//            }

//            bool diff_positions =
//                    (greater_than_or_equal_pt.x != less_than_pt.x) ||
//                    (greater_than_or_equal_pt.y != less_than_pt.y);

//            if(!diff_positions)
//            {
//                // The input hasn't changed
//                return false;
//            }

//            point = less_than_pt;

//            glm::vec2 position =
//                    InterpolateInputPoints(
//                        greater_than_or_equal_pt,
//                        less_than_pt,
//                        target_time);

//            point.x = position.x;
//            point.y = position.y;

//            return true;
//        }

//        static glm::vec2 InterpolateInputPoints(InputArea::Point const &a,
//                                                InputArea::Point const &b,
//                                                TimePoint const &t)
//        {
//            float total_dt = ks::CalcDuration<Milliseconds>(a.timestamp,b.timestamp).count();
//            float dt = ks::CalcDuration<Milliseconds>(a.timestamp,t).count();

//            glm::vec2 v;
//            v.x = (b.x-a.x)*dt/(total_dt) + a.x;
//            v.y = (b.y-a.y)*dt/(total_dt) + a.y;

//            return v;
//        }

//        void OnMouseEvent(ks::gui::MouseEvent mouse_event)
//        {
//            m_lkup_input_history[static_cast<uint>(InputType::Mouse)].
//                    push_back(ConvertToInputPoint(mouse_event));
//        }

//        void OnTouchEvent(ks::gui::TouchEvent touch_event)
//        {
//            // We don't currently support more than 3 touch inputs
//            if(touch_event.index > 2)
//            {
//                return;
//            }

//            uint const type_index =
//                    static_cast<uint>(touch_event.index+1);

//            m_lkup_input_history[type_index].push_back(
//                        ConvertToInputPoint(touch_event));
//        }

//        void OnAppResume()
//        {
//            rtklog.Trace() << "InputSystem On App Resume";
//            for(auto& list_input_frames : m_lkup_input_frames)
//            {
//                list_input_frames.clear();
//                list_input_frames.resize(m_input_buffer_size);
//            }

//            for(auto& input_history : m_lkup_input_history)
//            {
//                input_history.clear();
//            }
//        }
//    };

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
//        rtklog.Trace() << "INPUTSYSTEMUPDATE: " <<
//                          ks::CalcDuration<Milliseconds>(prev_upd_time,curr_upd_time).count();

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
