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

#include <raintk/RainTkMouseArea.hpp>
#include <raintk/RainTkScene.hpp>
#include <raintk/RainTkTransformSystem.hpp>
#include <raintk/RainTkLog.hpp>

#include <glm/glm.hpp>

namespace raintk
{
    MouseArea::MouseArea(ks::Object::Key const &key,
                         shared_ptr<Widget> parent,
                         std::string name) :
        InputArea(key,parent,name)
    {}

    void MouseArea::Init(ks::Object::Key const &,
                         shared_ptr<MouseArea> const &)
    {}

    MouseArea::~MouseArea()
    {}

    InputArea::Response MouseArea::handleInput(Point const &point)
    {
        if(point.type != Point::Type::Mouse)
        {
            return Response::Ignore;
        }

        bool const ignore_hover =
                (hover.Get()==false) &&
                (point.action == Point::Action::None) &&
                (point.button == Point::Button::None);

        if(ignore_hover)
        {
            return Response::Ignore;
        }

        Point curr_point = TransformPtToLocalCoords(point);

        bool notify_mouse=false;
        bool notify_entered=false;
        bool notify_exited=false;
        bool notify_pressed=false;
        bool notify_released=false;
        bool notify_clicked=false;

        bool const outside = PointOutside(curr_point,this);
        bool const inside = !outside;

        if(hover.Get())
        {
            if(inside)
            {
                notify_mouse = true;

                if(!m_lk_inside)
                {
                    notify_entered = true;
                }
            }
            else if(m_lk_inside)
            {
                notify_exited = true;
            }

            m_lk_inside = inside;
        }

        // Early ignore       
        if(outside && !m_inside_drag)
        {
            // Ignore notifying for duplicate points
//            if(curr_point.CompareIgnoreTime(mouse.Get())==false)
//            {
                if(notify_mouse)
                {
                    mouse = curr_point;
                }
                if(notify_entered)
                {
                    signal_entered.Emit();
                }
                if(notify_exited)
                {
                    signal_exited.Emit();
                }
//            }

            return Response::Ignore;
        }

        // Now we're either inside the MouseArea
        // or within a drag (or both)
        bool const mouse_pressed =
                (curr_point.action == Point::Action::Press);

        bool const mouse_released =
                (curr_point.action == Point::Action::Release);

        if(mouse_pressed && inside)
        {
            m_inside_drag = true;
            notify_mouse = true;
            notify_pressed = true;
        }
        else if(mouse_released)
        {
            if(m_inside_drag)
            {
                notify_mouse = true;
                notify_released = true;
                m_inside_drag = false;

                if(inside)
                {
                    notify_clicked = true;
                }
            }
            else
            {
                // The drag might be from another InputArea
                return Response::Ignore;
            }
        }

        // it cant be released so no need to check for that
        if(m_inside_drag && !mouse_pressed)
        {
            notify_mouse = true;
        }

        // Ignore notifying for duplicate points
//        if(curr_point.CompareIgnoreTime(mouse.Get())==false)
//        {
            if(notify_mouse)
            {
                mouse = curr_point;
            }
            if(notify_entered)
            {
                signal_entered.Emit();
            }
            if(notify_exited)
            {
                signal_exited.Emit();
            }
            if(notify_pressed)
            {
                signal_pressed.Emit(curr_point.button);
            }
            if(notify_released)
            {
                signal_released.Emit(curr_point.button);
            }
            if(notify_clicked)
            {
                signal_clicked.Emit(curr_point.button);
            }
//        }

        return Response::Accept;
    }

    void MouseArea::cancelInput(
            std::vector<Point> const &list_cancel_input)
    {
        for(auto const &cancel_pt : list_cancel_input)
        {
            Point local_pt = TransformPtToLocalCoords(cancel_pt);
            if(!PointOutside(local_pt,this))
            {
                if(m_inside_drag)
                {
                    // TODO, should we reassign point so that
                    // a signal_changed is emitted?
                    auto &pt = mouse.Get();
                    auto prev_button = pt.button;
                    pt.button = Point::Button::None;
                    pt.action = Point::Action::None;
                    // point.signal_changed.Emit();

                    signal_released.Emit(prev_button);

                    m_inside_drag = false;
                }
            }
        }
    }
}
