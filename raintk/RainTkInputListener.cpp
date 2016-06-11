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

#include <ks/gui/KsGuiApplication.hpp>

#include <raintk/RainTkInputListener.hpp>
#include <raintk/RainTkLog.hpp>

namespace raintk
{
    // ============================================================= //

    // Helpers
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

    InputListener::InputListener(ks::Object::Key const &key,
                                 ks::gui::Application* app) :
        ks::Object(key,app->GetEventLoop()),
        m_app(app),
        m_input_buffer_size(6),
        m_input_buffer_delay(2),
        m_frame(0),
        m_lkup_input_history(static_cast<uint>(InputType::TypeCount)),
        m_lkup_input_frames(static_cast<uint>(InputType::TypeCount))
    {
        m_lkup_input_frames[static_cast<uint>(InputType::Mouse)].resize(m_input_buffer_size);
        m_lkup_input_frames[static_cast<uint>(InputType::Touch0)].resize(m_input_buffer_size);
        m_lkup_input_frames[static_cast<uint>(InputType::Touch1)].resize(m_input_buffer_size);
        m_lkup_input_frames[static_cast<uint>(InputType::Touch2)].resize(m_input_buffer_size);
    }

    void InputListener::Init(ks::Object::Key const &,
                             shared_ptr<InputListener> const &this_listener)
    {
        m_app->signal_mouse_input->Connect(
                    this_listener,
                    &InputListener::onMouseEvent,
                    ks::ConnectionType::Direct);

        m_app->signal_touch_input->Connect(
                    this_listener,
                    &InputListener::onTouchEvent,
                    ks::ConnectionType::Direct);

        m_app->signal_keyboard_input->Connect(
                    this_listener,
                    &InputListener::onKeyEvent,
                    ks::ConnectionType::Direct);

        m_app->signal_utf8_input->Connect(
                    this_listener,
                    &InputListener::onUTF8Input,
                    ks::ConnectionType::Direct);

        m_app->signal_resume.Connect(
                    this_listener,
                    &InputListener::onAppResume,
                    ks::ConnectionType::Direct);
    }

    InputListener::~InputListener()
    {}

    std::vector<InputArea::Point>
    InputListener::GetInputs(TimePoint const &prev_upd_time,
                             TimePoint const &curr_upd_time)
    {
        // All input points for the delayed frame
        std::vector<InputArea::Point> list_all_points;

        uint const type_count = static_cast<uint>(InputType::TypeCount);
        for(uint type_idx=0; type_idx < type_count; type_idx++)
        {
            auto& input_frames = m_lkup_input_frames[type_idx];
            auto& input_history = m_lkup_input_history[type_idx];

            auto& oldest_frame = input_frames[0];

            // Trim input history and input frames
            trimHistoryBeforeFrame(input_history,oldest_frame);
            input_frames.erase(input_frames.begin());

            // Add new frame marking the end of any new
            // inputs received up until this point
            input_frames.emplace_back();
            auto& newest_frame = input_frames.back();
            newest_frame.t0 = prev_upd_time;
            newest_frame.t1 = curr_upd_time;
            newest_frame.num = m_frame;

            // Overwrite the position data for new input events
            // to reflect the target rendering time (close to the
            // end of the target frame) by interpolating
            auto& target_frame = input_frames[
                    m_input_buffer_size-m_input_buffer_delay];

            auto target_time = target_frame.t1 - Milliseconds(1);

            auto list_points =
                    collectPointsInFrame(
                        input_history,
                        target_frame);


            InputArea::Point ip_point;
            if(sampleInputHistory(input_history,target_time,ip_point))
            {
                if(list_points.empty())
                {
                    // Add an estimated input point if this frame has
                    // no inputs to make input position updating smoother
                    list_points.push_back(ip_point);
                }
                else
                {
                    for(auto& pt : list_points)
                    {
                        pt.x = ip_point.x;
                        pt.y = ip_point.y;
                    }
                }
            }

            // Save the points
            list_all_points.insert(
                        list_all_points.end(),
                        list_points.begin(),
                        list_points.end());
        }

        m_frame++;

        return list_all_points;
    }

    shared_ptr<Widget> InputListener::GetWidgetWithInputFocus() const
    {
        return m_focus_widget.lock();
    }

    void InputListener::SetInputFocus(shared_ptr<Widget> const &new_focus_widget)
    {
        auto curr_focus_widget = m_focus_widget.lock();

        if(curr_focus_widget &&
           curr_focus_widget != new_focus_widget)
        {
            curr_focus_widget->input_focus = false;
        }

        m_focus_widget = new_focus_widget;
    }

    void InputListener::ClearInputFocus()
    {
        auto curr_focus_widget = m_focus_widget.lock();
        if(curr_focus_widget)
        {
            m_focus_widget.reset();
        }
    }

    void InputListener::onAppResume()
    {
        rtklog.Trace() << "InputSystem On App Resume";
        for(auto& list_input_frames : m_lkup_input_frames)
        {
            list_input_frames.clear();
            list_input_frames.resize(m_input_buffer_size);
        }

        for(auto& input_history : m_lkup_input_history)
        {
            input_history.clear();
        }
    }

    void InputListener::onMouseEvent(ks::gui::MouseEvent mouse_event)
    {
        auto& input_history = m_lkup_input_history[static_cast<uint>(InputType::Mouse)];
        input_history.emplace_back();
        input_history.back().frame_num = m_frame;
        input_history.back().point = ConvertToInputPoint(mouse_event);
    }

    void InputListener::onTouchEvent(ks::gui::TouchEvent touch_event)
    {
        // We don't currently support more than 3 touch inputs
        if(touch_event.index > 2)
        {
            return;
        }

        uint const type_index = static_cast<uint>(touch_event.index+1);
        auto& input_history = m_lkup_input_history[type_index];

        input_history.emplace_back();
        input_history.back().frame_num = m_frame;
        input_history.back().point = ConvertToInputPoint(touch_event);
    }

    void InputListener::onKeyEvent(ks::gui::KeyEvent key_event)
    {
        auto focus_widget = m_focus_widget.lock();

        if(focus_widget)
        {
            focus_widget->handleKeyboardInput(key_event);
        }
    }

    void InputListener::onUTF8Input(std::string utf8text)
    {
        auto focus_widget = m_focus_widget.lock();

        if(focus_widget)
        {
            focus_widget->handleUTF8Input(utf8text);
        }
    }

    void InputListener::trimHistoryBeforeFrame(
            std::vector<HistoryPoint>& input_history,
            Frame const &frame)
    {
        input_history.erase(
                    std::remove_if(
                        input_history.begin(),
                        input_history.end(),
                        [&](HistoryPoint const &p){
                            return (p.frame_num < frame.num);
                        }),
                    input_history.end());
    }

    std::vector<InputArea::Point>
    InputListener::collectPointsInFrame(
            std::vector<HistoryPoint>& input_history,
            Frame const &frame)
    {
        std::vector<InputArea::Point> list_points;
        for(auto& p : input_history)
        {
            if(p.frame_num == frame.num)
            {
                list_points.push_back(p.point);
            }
        }

        return list_points;
    }

    bool InputListener::sampleInputHistory(
            std::vector<HistoryPoint>& input_history,
            TimePoint const &target_time,
            InputArea::Point& point)
    {
        point.timestamp = target_time;

        HistoryPoint compare_pt;
        compare_pt.point.timestamp = target_time;

        auto greater_than_or_equal_it =
                std::lower_bound(
                    input_history.begin(),
                    input_history.end(),
                    compare_pt,
                    [&](HistoryPoint const &a, HistoryPoint const &b){
                        return (a.point.timestamp < b.point.timestamp);
                    });

        if(greater_than_or_equal_it == input_history.end())
        {
            // No input points that occur after the target time
            return false;
        }

        auto& greater_than_or_equal_pt = *greater_than_or_equal_it;

        if(greater_than_or_equal_pt.point.timestamp == point.timestamp)
        {
            // There's already an Input point with the same timestamp
            point = greater_than_or_equal_pt.point;
            return true;
        }
        if(greater_than_or_equal_it == input_history.begin())
        {
            // The target time occurs before the earliest
            // known input history
            return false;
        }

        auto less_than_it = greater_than_or_equal_it;
        std::advance(less_than_it,-1);

        auto& less_than_pt = *less_than_it;

        if(less_than_pt.point.action == InputArea::Point::Action::Release)
        {
            // We can't interpolate as the input has ended
            return false;
        }

        bool diff_positions =
                (greater_than_or_equal_pt.point.x != less_than_pt.point.x) ||
                (greater_than_or_equal_pt.point.y != less_than_pt.point.y);

        if(!diff_positions)
        {
            // The input hasn't changed
            return false;
        }

        point = less_than_pt.point;
        point.timestamp = target_time;

        glm::vec2 position =
                interpolateInputPoints(
                    greater_than_or_equal_pt.point,
                    less_than_pt.point,
                    target_time);

        point.x = position.x;
        point.y = position.y;

        return true;
    }

    glm::vec2 InputListener::interpolateInputPoints(
            InputArea::Point const &a,
            InputArea::Point const &b,
            TimePoint const &t)
    {
        float total_dt = ks::CalcDuration<Milliseconds>(a.timestamp,b.timestamp).count();
        float dt = ks::CalcDuration<Milliseconds>(a.timestamp,t).count();

        glm::vec2 v;
        v.x = (b.x-a.x)*dt/(total_dt) + a.x;
        v.y = (b.y-a.y)*dt/(total_dt) + a.y;

        return v;
    }

    // ============================================================= //
}
