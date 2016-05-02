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

#ifndef RAINTK_SINGLE_POINT_AREA_HPP
#define RAINTK_SINGLE_POINT_AREA_HPP

#include <raintk/RainTkInputArea.hpp>

namespace raintk
{
    // SinglePointArea
    // * An InputArea widget for a single mouse
    //   or touch based InputPoint
    class SinglePointArea : public InputArea
    {
    public:
        using base_type = raintk::InputArea;

        SinglePointArea(ks::Object::Key const &key,
                        Scene* scene,
                        shared_ptr<Widget> parent);

        void Init(ks::Object::Key const &,
                  shared_ptr<SinglePointArea> const &);

        ~SinglePointArea();

        // Properties
        Property<InputArea::Point> point{
            Point{
                Point::Type::Mouse,
                Point::Button::None,
                Point::Action::None,
                std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max(),
                TimePoint(Milliseconds(0))
            }
        };

        // Signals
        ks::Signal<> signal_pressed;
        ks::Signal<> signal_released;
        ks::Signal<> signal_clicked;

    private:
        Response handleInput(Point const &,bool) override;
        void cancelInput() override;

        bool m_inside_drag{false};

    };
}

#endif // RAINTK_SINGLE_POINT_AREA_HPP
