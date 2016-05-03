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

#include <raintk/RainTkInputRecorder.hpp>
#include <raintk/RainTkInputArea.hpp>

namespace raintk
{
    namespace
    {
        std::string ConvertInputToString(
                uint frame,
                TimePoint const &frame_time,
                InputArea::Point const &point)
        {
            std::string line;
            line += ks::ToString(frame) + ",";
            line += ks::ToString(
                        static_cast<u64>(
                            std::chrono::duration_cast<Milliseconds>(
                                frame_time.time_since_epoch()).count())) + ",";
            line += ks::ToString(
                        static_cast<u64>(
                            std::chrono::duration_cast<Milliseconds>(
                                point.timestamp.time_since_epoch()).count())) + ",";
            line += ks::ToString(static_cast<uint>(point.type)) + ",";
            line += ks::ToString(static_cast<uint>(point.button)) + ",";
            line += ks::ToString(static_cast<uint>(point.action)) + ",";
            line += ks::ToString(static_cast<float>(point.x)) + ",";
            line += ks::ToString(static_cast<float>(point.y)) + "\n";

            return line;
        }

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


    InputRecorder::InputRecorder(ks::Object::Key const &key,
                                 ks::gui::Application* app,
                                 std::string file_path,
                                 uint max_frames) :
        ks::Object(key,app->GetEventLoop()),
        m_app(app),
        m_file_path(file_path),
        m_max_frames(max_frames),
        m_frame(0),
        m_needs_commit(false)
    {}

    void InputRecorder::Init(ks::Object::Key const &,
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

        m_cid_app_pause =
                m_app->signal_pause.Connect(
                    this_recorder,
                    &InputRecorder::onAppPause);
    }

    InputRecorder::~InputRecorder()
    {
        commitToFile();
    }

    void InputRecorder::Update(TimePoint const &curr_upd_time)
    {
        m_frame++;
        m_frame_time = curr_upd_time;

        if(m_list_input_data.size() > m_max_frames)
        {
            auto const first_frame = m_list_input_data.front().first;

            m_list_input_data.erase(
                        std::remove_if(
                                    m_list_input_data.begin(),
                                    m_list_input_data.end(),
                                    [&](std::pair<uint,std::string> const &e){
                                        return (e.first == first_frame);
                                    }),
                        m_list_input_data.end());
        }
    }

    void InputRecorder::onMouseEvent(ks::gui::MouseEvent mouse_event)
    {
        m_list_input_data.emplace_back(
                    m_frame,
                    ConvertInputToString(
                        m_frame,
                        m_frame_time,
                        ConvertToInputPoint(mouse_event)));

        m_needs_commit = true;
    }

    void InputRecorder::onTouchEvent(ks::gui::TouchEvent touch_event)
    {
        m_list_input_data.emplace_back(
                    m_frame,
                    ConvertInputToString(
                        m_frame,
                        m_frame_time,
                        ConvertToInputPoint(touch_event)));

        m_needs_commit = true;
    }

    void InputRecorder::onAppPause()
    {
        commitToFile();
    }

    void InputRecorder::commitToFile()
    {
        if(m_needs_commit)
        {
            std::ofstream file(m_file_path, std::ios_base::out | std::ios_base::trunc);
            for(auto& frame_line : m_list_input_data)
            {
                file << frame_line.second;
            }

            m_needs_commit = false;
        }
    }
}
