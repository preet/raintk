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

#ifndef RAINTK_MOUSE_AREA_HPP
#define RAINTK_MOUSE_AREA_HPP

#include <raintk/RainTkInputArea.hpp>

namespace raintk
{
    // MouseArea
    // * An InputArea widget for a single mouse InputPoint
    class MouseArea : public InputArea
    {
    public:
        using base_type = raintk::InputArea;

        MouseArea(ks::Object::Key const &key,
                  shared_ptr<Widget> parent,
                  std::string name);

        void Init(ks::Object::Key const &,
                  shared_ptr<MouseArea> const &);

        ~MouseArea();

        // Properties
        Property<InputArea::Point> mouse{
            name+".mouse",
            Point{
                Point::Type::Mouse,
                Point::Button::None,
                Point::Action::None,
                std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max(),
                TimePoint(Milliseconds(0))
            }
        };

        Property<bool> hover{
            name+".hover",false
        };

        // Signals
        ks::Signal<> signal_entered;
        ks::Signal<> signal_exited;
        ks::Signal<Point::Button> signal_pressed;
        ks::Signal<Point::Button> signal_released;
        ks::Signal<Point::Button> signal_clicked;

    private:
        Response handleInput(Point const &) override;
        void cancelInput(std::vector<Point> const &) override;

        bool m_inside_drag{false};
        bool m_lk_inside{false};
    };
}

#endif // RAINTK_MOUSE_AREA_HPP
