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
#include <raintk/RainTkGlobal.hpp>

namespace raintk
{
    class InputRecorder : public ks::Object
    {
    public:
        using base_type = ks::Object;

        InputRecorder(ks::Object::Key const &key,
                      ks::gui::Application* app,
                      std::string file_path,
                      uint max_frames=3600);

        void Init(ks::Object::Key const &,
                  shared_ptr<InputRecorder> const &);

        ~InputRecorder();

        void Update(TimePoint const &curr_upd_time);


    private:
        void onMouseEvent(ks::gui::MouseEvent mouse_event);
        void onTouchEvent(ks::gui::TouchEvent touch_event);
        void onAppPause();
        void commitToFile();


        ks::gui::Application* const m_app;
        std::string const m_file_path;
        uint const m_max_frames;

        uint m_frame;
        TimePoint m_frame_time;

        Id m_cid_mouse_events;
        Id m_cid_touch_events;
        Id m_cid_app_pause;

        bool m_needs_commit;
        std::vector<std::pair<uint,std::string>> m_list_input_data;
    };
}
