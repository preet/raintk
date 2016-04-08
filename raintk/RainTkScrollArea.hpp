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

#ifndef RAINTK_SCROLL_AREA_HPP
#define RAINTK_SCROLL_AREA_HPP

#include <raintk/RainTkSinglePointArea.hpp>

namespace raintk
{
    // =========================================================== //

    class InvalidScrollAreaContentParent : public ks::Exception
    {
    public:
        InvalidScrollAreaContentParent(std::string msg);
        ~InvalidScrollAreaContentParent() = default;
    };

    class InvalidScrollAreaAcceleration : public ks::Exception
    {
    public:
        InvalidScrollAreaAcceleration(std::string msg);
        ~InvalidScrollAreaAcceleration() = default;
    };

    class ScrollFlickAnimation;

    // =========================================================== //

    // ScrollArea
    // * This widget's area can be scrolled and panned
    //   to move its child widgets correspondingly
    // * If clip is true, child widgets will be clipped
    //   by the ScrollView's bounds
    class ScrollArea : public InputArea
    {
    public:
        using base_type = raintk::InputArea;

        enum class Direction : u8
        {
            Horizontal              = 1 << 0,
            Vertical                = 1 << 1,
            HorizontalAndVertical   = 3 // (Horizontal | Vertical)
        };


        ScrollArea(ks::Object::Key const &key,
                   shared_ptr<Widget> parent,
                   std::string name);

        void Init(ks::Object::Key const &,
                  shared_ptr<ScrollArea> const &);

        ~ScrollArea();

        // Children must be added to the Content Parent
        // and *not* the ScrollView
        shared_ptr<Widget> GetContentParent() const;

        Milliseconds GetFlickStopDuration() const;
        float GetMinimumFlickSpeed() const;
        float GetDeceleration() const;

        // * Set the minimum stop duration required to prevent
        //   a flick from occuring
        void SetFlickStopDuration(Milliseconds min_stop_duration_ms);

        // * Set the minimum speed required for a flick animation
        //   in meters/second
        void SetMinimumFlickSpeed(float min_flick_speed_m_s);

        // * Set the deceleration rate at which the flick
        //   animation will slow down in (m/s^2)
        void SetDeceleration(float deceleration_m_s_2);

        // Clamp the content position to the view extents.
        // If the width/height of the content is smaller
        // than the width/height of the view, x/y will
        // be set to 0
        void ClampContentToView();

        // Trying to add children directly to this Widget
        // will throw an exception
        void AddChild(shared_ptr<Widget> const &child) override;


        // Properties
        Property<Direction> direction{
            name+".direction",Direction::HorizontalAndVertical
        };

        // Sets whether or not a continued scrolling animation
        // occurs after input activity has stopped
        Property<bool> flick{
            name+".flick",true
        };


    protected:
        // The content parent widget that contains all widgets
        // that should be manipulated by this ScrollArea
        shared_ptr<Widget> m_content_parent;

    private:
        void onStartScrolling();
        void onStopScrolling();
        void onContinueScrolling();
        void onScrollMotion(glm::vec2 const &v);
        bool verifyScrollStart();
        Response handleInput(Point const &new_point) override;
        Response handleSinglePointInput(Point const &new_point);
        void cancelInput(std::vector<Point> const &) override;

        bool m_inside_drag{false};
        bool m_inside_scroll{false};
        float const m_scroll_threshold_mm{mm(2.0f)};

        Milliseconds m_min_stop_duration_ms{Milliseconds(65)};
        float m_min_flick_speed_m_s{0.0f};
        float m_deceleration_m_s_2{1.5f};

        float m_scroll_capture_angle_degs{20};

        // The most recent drag point (orthogonal to whether
        // or not this widget is currently scrolling)
        Point m_point{
            Point::Type::Mouse,
            Point::Button::None,
            Point::Action::None,
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
            TimePoint(Milliseconds(0))
        };

        // The point at which a drag started; used to keep track
        // of how much distance the drag has covered
        Point m_drag_start_point;

        // The list of points that are within a valid scroll.
        // The most recent points are at the end of the list
        // and the list is cleared when a scroll motion ends.
        std::vector<InputArea::Point> m_list_scroll_points;

        // A history of points we save to determine which
        // InputAreas to cancel
        std::vector<InputArea::Point> m_list_cancel_history;

        // The currently active flick animation (if any)
        shared_ptr<ScrollFlickAnimation> m_flick_anim;
    };

    // =========================================================== //
}

#endif // RAINTK_SCROLL_AREA_HPP
