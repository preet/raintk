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

#include <raintk/RainTkGlobal.hpp>
#include <raintk/RainTkInputArea.hpp>
#include <ks/gui/KsGuiInput.hpp>

namespace ks
{
    namespace gui
    {
        class Application;
    }
}

namespace raintk
{
    class InputListener : public ks::Object
    {
        struct Frame
        {
            u64 num;
            TimePoint t0{Milliseconds(0)};
            TimePoint t1{Milliseconds(0)};
        };

        struct HistoryPoint
        {
            u64 frame_num;
            InputArea::Point point;
        };

        using InputType = InputArea::Point::Type;

    public:
        using base_type = ks::Object;

        InputListener(ks::Object::Key const &key,
                      ks::gui::Application* app);

        void Init(ks::Object::Key const &,
                  shared_ptr<InputListener> const &);

        ~InputListener();

        // Should be called every frame
        std::vector<InputArea::Point>
        GetInputs(TimePoint const &prev_upd_time,
                  TimePoint const &curr_upd_time);


        // Set the widget that receives non-area inputs
        void SetInputFocus(shared_ptr<Widget> const &focus_widget);
        void ClearInputFocus();

    private:
        void onAppResume();
        void onMouseEvent(ks::gui::MouseEvent mouse_event);
        void onTouchEvent(ks::gui::TouchEvent touch_event);

        // These are pass through events
        void onKeyEvent(ks::gui::KeyEvent key_event);
        void onUTF8Input(std::string utf8text);

        static void trimHistoryBeforeFrame(
                std::vector<HistoryPoint>& input_history,
                Frame const &frame);

        static std::vector<InputArea::Point> collectPointsInFrame(
                std::vector<HistoryPoint>& input_history,
                Frame const &frame);

        static bool sampleInputHistory(
                std::vector<HistoryPoint>& input_history,
                TimePoint const &target_time,
                InputArea::Point& point);

        static glm::vec2 interpolateInputPoints(
                InputArea::Point const &a,
                InputArea::Point const &b,
                TimePoint const &t);


        ks::gui::Application* m_app;

        // input buffer size in frames
        uint m_input_buffer_size;

        // input buffer delay in frames, must be less than buffer size
        uint m_input_buffer_delay;

        // current frame, incremented when GetInputs is called
        u64 m_frame;

        // lookup lists indexed by InputType
        std::vector<std::vector<HistoryPoint>> m_lkup_input_history;
        std::vector<std::vector<Frame>> m_lkup_input_frames;

        // widget with current input focus for non area inputs
        shared_ptr<Widget> m_focus_widget;
    };
}
