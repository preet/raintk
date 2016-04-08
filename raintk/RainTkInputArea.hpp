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

#ifndef RAINTK_INPUT_AREA_HPP
#define RAINTK_INPUT_AREA_HPP

#include <array>
#include <raintk/RainTkWidget.hpp>

namespace raintk
{
    class InputSystem;

    // InputArea is the base class for:
    // - MouseArea [single point, multiple mouse buttons]
    // - PointerArea [single or multiple points, single 'button']
    class InputArea : public Widget
    {
        friend class InputSystem;

    public:
        using base_type = raintk::Widget;

        // Point contains both motion and button state
        // for a given input point
        struct Point
        {
            enum class Button : u8
            {
                None    = 0,
                Left    = (1 << 0),
                Middle  = (1 << 1),
                Right   = (1 << 2)
            };

            enum class Action : u8
            {
                None,
                Press,
                Release
            };

            enum class Type : u8
            {
                Mouse  = 0,
                Touch0 = 1,
                Touch1 = 2,
                Touch2 = 3,
                TypeCount = 4
            };

            Type type;
            Button button;
            Action action;
            float x;
            float y;
            TimePoint timestamp;

            bool CompareIgnoreTime(Point const &other) const
            {
                bool different =
                        (other.type != type) ||
                        (other.button != button) ||
                        (other.action != action) ||
                        (other.x != x) ||
                        (other.y != y);

                return !different;
            }

            bool CompareWithTime(Point const &other) const
            {
                bool different =
                        (other.timestamp == timestamp) ||
                        (other.type != type) ||
                        (other.button != button) ||
                        (other.action != action) ||
                        (other.x != x) ||
                        (other.y != y);

                return !different;
            }


        }; // size = 10

        //
        enum class Response : u8
        {
            Accept,
            Ignore
        };

        InputArea(ks::Object::Key const &key,
                  shared_ptr<Widget> parent,
                  std::string name);

        void Init(ks::Object::Key const &,
                  shared_ptr<InputArea> const &);

        ~InputArea();

        // Determines whether or not an InputArea::Point
        // is outside of the Widget

        // NOTE: Expects @point to be in widget-local
        // coordinates
        static bool PointOutside(Point const &point,
                                 Widget* input_area)
        {
            return point.x < 0.0f || point.x > input_area->width.Get() ||
                   point.y < 0.0f || point.y > input_area->height.Get();
        }

        // Transform @point to be in this widget's local
        // coordinate space
        Point TransformPtToLocalCoords(Point const &point) const;

        // Properties

        // The InputArea only receives input if it is enabled
        Property<bool> enabled{
            name+".enabled",true
        };

    protected:
        static void cancelInputsBehindDepth(
                InputSystem* input_system,
                std::vector<Point> const &list_points,
                float depth);

        virtual void onEnabledChanged();

        virtual Response handleInput(Point const &) = 0;
        virtual void cancelInput(std::vector<Point> const &) = 0;

        Id m_cid_enabled;

    private:
        InputDataComponentList* const m_cmlist_input_data;
    };
}

#endif // RAINTK_INPUT_AREA_HPP
