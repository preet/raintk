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

#include <raintk/RainTkScrollArea.hpp>
#include <raintk/RainTkAnimation.hpp>
#include <raintk/RainTkScene.hpp>
#include <raintk/RainTkInputSystem.hpp>
#include <raintk/RainTkLog.hpp>

#include <glm/glm.hpp>

namespace raintk
{
    // =========================================================== //

    InvalidScrollAreaContentParent::InvalidScrollAreaContentParent(std::string msg) :
        ks::Exception(ks::Exception::ErrorLevel::ERROR,msg)
    {}

    InvalidScrollAreaAcceleration::InvalidScrollAreaAcceleration(std::string msg) :
        ks::Exception(ks::Exception::ErrorLevel::ERROR,msg)
    {}

    // =========================================================== //

    // ScrollFlickAnimation
    // * Animation for flicking ScrollAreas
    // * Uses kinematic equations for uniform acceleration
    class ScrollFlickAnimation : public Animation
    {
    public:
        using base_type = raintk::Animation;

        ScrollFlickAnimation(ks::Object::Key const &key,
                             Scene* scene,
                             std::function<void(glm::vec2 const &)> update_pos_callback,
                             glm::vec2 dirn_unit_vector,
                             float initial_velocity,
                             float acceleration) :
            Animation(key,scene),

            // Callback to invoke for (relative) position updates
            m_upd_position(update_pos_callback),

            // The direction vector for the velocity (normalized)
            m_unit_vec(dirn_unit_vector),

            // The initial velocity must be greater than 0,
            // specified in units/s
            m_speed_i_m_s(initial_velocity),

            // The acceleration must be less than 0,
            // specified in units/s^2
            m_accel_m_s2(acceleration),

            // Determine the duration of time it takes to
            // decelerate to a final velocity of zero (seconds)
            m_interval_s((-1*m_speed_i_m_s)/m_accel_m_s2)
        {
            if(!(m_accel_m_s2 < 0))
            {
                throw InvalidScrollAreaAcceleration(
                            "ScrollFlickAnimation acceleration must be "
                            "less than zero");
            }
        }

        void Init(ks::Object::Key const &,
                  shared_ptr<ScrollFlickAnimation> const &)
        {}

        ~ScrollFlickAnimation()
        {}

        void start() override
        {
            m_position_m = 0;
            m_time_s = 0;
        }

        bool update(float delta_ms) override
        {
            float delta_s = delta_ms/1000.0f;

            m_time_s = std::min(m_time_s+delta_s,m_interval_s);
            m_speed_m_s = m_speed_i_m_s + m_accel_m_s2*m_time_s;

            auto new_position_m = 0.5f*(m_speed_i_m_s+m_speed_m_s)*m_time_s;
            auto position_change_mm = m_unit_vec*(new_position_m-m_position_m)*1000.0f;

            m_upd_position(position_change_mm);
            m_position_m = new_position_m;

            if(m_time_s == m_interval_s)
            {
                return true; // completed
            }

            return false;
        }

        void complete() override
        {}

    private:
        std::function<void(glm::vec2 const &)> m_upd_position;
        glm::vec2 const m_unit_vec;
        float const m_speed_i_m_s; // initial speed
        float const m_accel_m_s2;
        float const m_interval_s;
        float m_speed_m_s; // accumulated speed (m/s)
        float m_position_m; // accumulated position (m)
        float m_time_s; // accumulated time (s)
    };

    // =========================================================== //

    ScrollArea::ScrollArea(ks::Object::Key const &key,
                           shared_ptr<Widget> parent,
                           std::string name) :
        InputArea(key,parent,name)
    {}

    void ScrollArea::Init(ks::Object::Key const &,
                          shared_ptr<ScrollArea> const &this_scroll_area)
    {
        // Create the content parent
        m_content_parent =
                MakeWidget<Widget>(
                    this_scroll_area,
                    name+".content_parent");

        // Set some default content dimensions
        m_content_parent->width = mm(250);
        m_content_parent->height = mm(250);

        // Setup connections

        // Ensure that any dimension changes for this
        // widget or the content parent keep the content
        // position within bounds
        m_content_parent->height.signal_changed.Connect(
                    this_scroll_area,
                    &ScrollArea::checkContentPositionChange,
                    ks::ConnectionType::Direct);

        m_content_parent->width.signal_changed.Connect(
                    this_scroll_area,
                    &ScrollArea::checkContentPositionChange,
                    ks::ConnectionType::Direct);

        height.signal_changed.Connect(
                    this_scroll_area,
                    &ScrollArea::checkContentPositionChange,
                    ks::ConnectionType::Direct);

        width.signal_changed.Connect(
                    this_scroll_area,
                    &ScrollArea::checkContentPositionChange,
                    ks::ConnectionType::Direct);
    }

    ScrollArea::~ScrollArea()
    {}

    shared_ptr<Widget> ScrollArea::GetContentParent() const
    {
        return m_content_parent;
    }

    Milliseconds ScrollArea::GetFlickStopDuration() const
    {
        return m_min_stop_duration_ms;
    }

    float ScrollArea::GetMinimumFlickSpeed() const
    {
        return m_min_flick_speed_m_s;
    }

    float ScrollArea::GetDeceleration() const
    {
        return m_deceleration_m_s_2;
    }

    void ScrollArea::SetFlickStopDuration(Milliseconds min_stop_duration_ms)
    {
        m_min_stop_duration_ms = min_stop_duration_ms;
    }

    void ScrollArea::SetMinimumFlickSpeed(float min_flick_speed_m_s)
    {
        m_min_flick_speed_m_s = min_flick_speed_m_s;
    }

    void ScrollArea::SetDeceleration(float deceleration_m_s_2)
    {
        if(!(deceleration_m_s_2 > 0))
        {
            throw InvalidScrollAreaAcceleration(
                        "ScrollArea deceleration must be "
                        "greater than zero");
        }

        m_deceleration_m_s_2 = deceleration_m_s_2;
    }

    void ScrollArea::ClampContentToView()
    {
        // x
        {
            auto const content_width = m_content_parent->width.Get();
            auto const view_width = this->width.Get();
            if(content_width > view_width)
            {
                if(m_content_parent->x.Get() > 0.0f)
                {
                    m_content_parent->x = 0.0f;
                }
                else if(m_content_parent->x.Get() < (view_width-content_width))
                {
                    m_content_parent->x = (view_width-content_width);
                }
            }
            else
            {
                if(m_content_parent->x.Get() != 0.0f)
                {
                    m_content_parent->x = 0.0f;
                }
            }
        }

        // y
        {
            auto const content_height = m_content_parent->height.Get();
            auto const view_height = this->height.Get();

            if(content_height > view_height)
            {
                if(m_content_parent->y.Get() > 0.0f)
                {
                    m_content_parent->y = 0.0f;
                }
                else if(m_content_parent->y.Get() < (view_height-content_height))
                {
                    m_content_parent->y = (view_height-content_height);
                }
            }
            else
            {
                if(m_content_parent->y.Get() != 0.0f)
                {
                    m_content_parent->y = 0.0f;
                }
            }
        }
    }

    void ScrollArea::SetContentX(float new_x)
    {
        auto const delta_x = new_x-m_content_parent->x.Get();

        requestContentPositionChange(
                    glm::vec2(delta_x,0.0f));
    }

    void ScrollArea::SetContentY(float new_y)
    {
        auto const delta_y = new_y-m_content_parent->y.Get();

        requestContentPositionChange(
                    glm::vec2(0.0f,delta_y));
    }

    void ScrollArea::SetContentPosition(float new_x, float new_y)
    {
        auto const delta_x = new_x-m_content_parent->x.Get();
        auto const delta_y = new_y-m_content_parent->y.Get();

        requestContentPositionChange(
                    glm::vec2(delta_x,delta_y));
    }

    void ScrollArea::AddChild(shared_ptr<Widget> const &child)
    {
        bool const is_content_parent =
                GetChildren().empty() &&
                (child->name == (name+".content_parent"));

        if(!is_content_parent)
        {
            throw InvalidScrollAreaContentParent(
                        "ScrollArea child widgets must be parented "
                        "to its ContentParent: "+child->name);
        }
        else
        {
            Widget::AddChild(child);
        }
    }

    void ScrollArea::onStartScrolling()
    {
        m_list_scroll_points.push_back(m_point);
    }

    void ScrollArea::onStopScrolling()
    {
        // The last point where the input is released generally
        // has the same x,y as the point before it so we ignore it:
        // m_list_scroll_points.push_back(m_point);

        // Handle stop -- ie animate if continued motion req'd
        if(flick.Get())
        {
            if(m_list_scroll_points.size() > 1)
            {
                // Figure out how long the duration between the
                // time we stopped scrolling (when m_point was set)
                // and the last point that represented motion was
                float stop_duration_ms =
                        ks::CalcDuration<Microseconds>(
                            m_list_scroll_points.back().timestamp,
                            m_point.timestamp).
                        count()/1000.0f;

                if(stop_duration_ms < m_min_stop_duration_ms.count())
                {
                    // For the flick velocity, use the average velocity
                    // over the interval of saved scroll points
                    auto const &p0 = m_list_scroll_points.front(); // first
                    auto const &p1 = m_list_scroll_points.back();  // last

                    float flick_interval_ms =
                            ks::CalcDuration<Microseconds>(
                                p0.timestamp,
                                p1.timestamp).
                            count()/1000.0f;

                    // Flick velocity is in m/s (mm/ms == m/s)
                    glm::vec2 flick_velocity(
                                (p1.x-p0.x)/flick_interval_ms,
                                (p1.y-p0.y)/flick_interval_ms);

                    if(direction.Get() == Direction::Horizontal)
                    {
                        flick_velocity.y = 0.0f;
                    }
                    else if(direction.Get() == Direction::Vertical)
                    {
                        flick_velocity.x = 0.0f;
                    }

                    // If the velocity exceeds a certain threshold,
                    // create a scrolling animation
                    // TODO if(...)
                    m_flick_anim =
                            ks::MakeObject<ScrollFlickAnimation>(
                                m_scene,
                                [this](glm::vec2 const &v){ requestContentPositionChange(v); },
                                glm::normalize(flick_velocity),
                                glm::length(flick_velocity),
                                m_deceleration_m_s_2*-1.0f);

                    m_flick_anim->Start();
                }
            }
        }

        m_list_scroll_points.clear();
    }

    void ScrollArea::onContinueScrolling()
    {
        m_list_scroll_points.push_back(m_point);

        // With a size of 6 we probably average over
        // 2 to 3 updates a frame
        if(m_list_scroll_points.size() > 6)
        {
            m_list_scroll_points.erase(
                        m_list_scroll_points.begin());
        }

        // As long as a point is added to list_scroll_points in
        // onStartScrolling, we should always have at least 2 points
        auto const &tp0 = *(std::prev(m_list_scroll_points.end(),2));
        auto const &tp1 = *(std::prev(m_list_scroll_points.end(),1));

        glm::vec2 ds((tp1.x-tp0.x),(tp1.y-tp0.y));

        requestContentPositionChange(ds);
    }

    void ScrollArea::checkContentPositionChange()
    {
        requestContentPositionChange(glm::vec2(0.0f,0.0f));
    }

    void ScrollArea::requestContentPositionChange(glm::vec2 const &v)
    {
        u8 const direction_val = u8(direction.Get());

        bool x_edge_reached = false;
        bool y_edge_reached = false;

        if((u8(Direction::Horizontal) & direction_val) > 0)
        {
            auto const this_width = width.Get();
            auto const content_width = m_content_parent->width.Get();

            if(content_width > this_width)
            {
                float new_x = v.x + m_content_parent->x.Get();

                if(new_x > 0)
                {
                    new_x = 0; // left edge
                    x_edge_reached = true;
                }
                else if(new_x < (this_width-content_width))
                {
                    new_x = this_width-content_width; // right edge
                    x_edge_reached = true;
                }

                m_content_parent->x = new_x;
            }
            else
            {
                if(m_content_parent->x.Get() != 0.0f)
                {
                    m_content_parent->x = 0.0f;
                }
            }
        }
        if((u8(Direction::Vertical) & direction_val) > 0)
        {
            auto const this_height = height.Get();
            auto const content_height = m_content_parent->height.Get();

            if(content_height > this_height)
            {
                float new_y = v.y + m_content_parent->y.Get();

                if(new_y > 0)
                {
                    new_y = 0; // top edge
                    y_edge_reached = true;
                }
                else if(new_y < (this_height-content_height))
                {
                    new_y = this_height-content_height; // bottom edge
                    y_edge_reached = true;
                }

                m_content_parent->y = new_y;
            }
            else
            {
                if(m_content_parent->y.Get() != 0.0f)
                {
                    m_content_parent->y = 0.0f;
                }
            }
        }


        if(m_flick_anim)
        {
            if((x_edge_reached && v.y==0) || (y_edge_reached && v.x==0))
            {
                // Stop the animation if either bound has been reached.
                // This also allows input events to pass through to widgets
                // below without having to wait for the animation to stop
                m_flick_anim->Stop();
                m_flick_anim->Remove();
            }
        }
    }

    InputArea::Response ScrollArea::handleInput(Point const &new_point)
    {
        bool const prev_inside_drag = m_inside_drag;

        auto response = handleSinglePointInput(new_point);

        if(response == Response::Ignore)
        {
            return response;
        }

        if(prev_inside_drag && m_inside_drag==false)
        {
            m_list_cancel_history.clear();

            // Ended the drag
            if(m_inside_scroll)
            {
                m_inside_scroll = false;
                onStopScrolling();
            }
            else
            {
                // Pass through if a scroll was never initiated
                return Response::Ignore;
            }
            return Response::Accept;
        }

        if(m_inside_drag && prev_inside_drag==false)
        {
            // Started a drag
            m_drag_start_point = m_point;

            if(m_flick_anim &&
               m_flick_anim->GetState()==Animation::State::Running)
            {
                m_flick_anim->Stop();
                m_flick_anim->Remove();

                // We used the input to stop the flick anim
                // so it shouldn't be used for anything else
                return Response::Accept;
            }

            // Pass through as the scroll hasn't started yet
            // Cancel history has to save input points
            // in global coordinates
            m_list_cancel_history.push_back(new_point);
            return Response::Ignore;
        }
        else
        {
            // Continuing a drag
            if(!m_inside_scroll)
            {
                if(verifyScrollStart())
                {
                    // Cancel any InputAreas that may have
                    // been active from pass through
                    auto const this_depth =
                            m_cmlist_xf_data->GetComponent(
                                m_entity_id).world_xf[3].z;

                    InputArea::cancelInputsBehindDepth(
                                m_scene->GetInputSystem(),
                                m_list_cancel_history,
                                this_depth);

                    m_list_cancel_history.clear();


                    // Scroll threshold reached
                    m_inside_scroll = true;
                    onStartScrolling();
                    return Response::Accept;
                }
                // Cancel history has to save input points
                // in global coordinates
                m_list_cancel_history.push_back(new_point);
            }
            else
            {
                onContinueScrolling();
                return Response::Accept;
            }
        }

        // Pass-through to InputAreas below this one
        return Response::Ignore;
    }

    bool ScrollArea::verifyScrollStart()
    {
        auto const &p0 = m_drag_start_point;
        auto const &p1 = m_point;
        float dx = p0.x-p1.x;
        float dy = p0.y-p1.y;
        float dist = sqrt(dx*dx + dy*dy);
        // rtklog.Trace() << "dist: " << dist;

        if(dist >= m_scroll_threshold_mm)
        {
            float angle_degs = atan2(dy,dx)*180.0f/k_pi;

            // Capture / Start scrolling only if the direction
            // of the drag matches the direction of the ScrollArea
            if(direction.Get() == Direction::Horizontal)
            {
                float lim_a = m_scroll_capture_angle_degs;
                float lim_b = 180.0f-m_scroll_capture_angle_degs;

                float lim_c = m_scroll_capture_angle_degs*-1.0f;
                float lim_d = m_scroll_capture_angle_degs-180.0f;

                bool outside =
                        (angle_degs > lim_a && angle_degs < lim_b) ||
                        (angle_degs < lim_c && angle_degs > lim_d);

                return !outside;
            }
            else if(direction.Get() == Direction::Vertical)
            {
                float lim_a = 90.0f-m_scroll_capture_angle_degs;
                float lim_b = 90.0f+m_scroll_capture_angle_degs;

                float lim_c = -90.0f+m_scroll_capture_angle_degs;
                float lim_d = -90.0f-m_scroll_capture_angle_degs;

                bool inside =
                        (angle_degs >= lim_a && angle_degs <= lim_b) ||
                        (angle_degs <= lim_c && angle_degs >= lim_d);

                return inside;
            }

            // No need to check if direction is both H and V
            return true;
        }

        return false;
    }

    InputArea::Response ScrollArea::handleSinglePointInput(Point const &new_point)
    {
        Point curr_point = TransformPtToLocalCoords(new_point);
        bool const outside = PointOutside(curr_point,this);
        bool const inside = !outside;

        bool notify_point=false;

        Response response = Response::Ignore;

        // State: not inside drag (ignoring)
        if(!m_inside_drag)
        {
            // Transition: Press within area
            if(curr_point.action == Point::Action::Press && inside)
            {
                m_inside_drag = true;
                notify_point = true;
                response = Response::Accept;
            }
        }
        // State: inside drag (pressed/dragging)
        else
        {
            if(curr_point.action == Point::Action::Release)
            {
                if(inside)
                {
                    // Transition: Release within area
                }
                // else
                // {
                    // Transition: Release outside of area
                // }

                m_inside_drag = false;
                notify_point = true;
            }
            else
            {
                notify_point = true;
            }

            response = Response::Accept;
        }

        if(notify_point)
        {
            m_point = curr_point;
        }

        return response;
    }

    void ScrollArea::cancelInput(
            std::vector<Point> const &list_cancel_pts)
    {
        for(auto const &cancel_pt : list_cancel_pts)
        {
            Point local_pt = TransformPtToLocalCoords(cancel_pt);
            if(!PointOutside(local_pt,this))
            {
                if(m_inside_scroll)
                {
                    m_inside_scroll = false;
                    onStopScrolling();
                    m_flick_anim->Stop();
                    m_flick_anim->Remove();
                }

                if(m_inside_drag)
                {
                    m_point.button = Point::Button::None;
                    m_point.action = Point::Action::None;
                    m_inside_drag = false;
                }
                break;
            }
        }
    }

    // =========================================================== //
}
