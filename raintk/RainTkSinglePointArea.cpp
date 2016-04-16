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

#include <raintk/RainTkSinglePointArea.hpp>
#include <raintk/RainTkScene.hpp>
#include <raintk/RainTkLog.hpp>

#include <glm/glm.hpp>

namespace raintk
{
    SinglePointArea::SinglePointArea(ks::Object::Key const &key,
                                     Scene* scene,
                                     shared_ptr<Widget> parent) :
        InputArea(key,scene,parent)
    {}

    void SinglePointArea::Init(ks::Object::Key const &,
                               shared_ptr<SinglePointArea> const &)
    {}

    SinglePointArea::~SinglePointArea()
    {}

    InputArea::Response
    SinglePointArea::handleInput(Point const &new_point,
                                 bool inside)
    {
        bool notify_point=false;
        bool notify_pressed=false;
        bool notify_released=false;
        bool notify_clicked=false;

        Response response = Response::Ignore;


        if(!m_inside_drag)
        {
            // Transition: Press within area
            if(new_point.action == Point::Action::Press && inside)
            {
                m_inside_drag = true;
                notify_point = true;
                notify_pressed = true;
                response = Response::Accept;
            }
        }
        else
        {
            if(new_point.action == Point::Action::Release)
            {
                if(inside)
                {
                    // Transition: Release within area
                    notify_clicked = true;
                }
                // else
                // {
                    // Transition: Release outside of area
                // }

                notify_point = true;
                m_inside_drag = false;
                notify_released = true;
            }
            else
            {
                notify_point = true;
            }

            response = Response::Accept;
        }

        // Early exit
        if(response == Response::Ignore)
        {
            return response;
        }

        if(notify_point)
        {
            point = new_point;
        }
        if(notify_pressed)
        {
            signal_pressed.Emit();
        }
        if(notify_released)
        {
            signal_released.Emit();
        }
        if(notify_clicked)
        {
            signal_clicked.Emit();
        }

        return response;
    }

    void SinglePointArea::cancelInput()
    {
        if(m_inside_drag)
        {
            // TODO, should we reassign point so that
            // a signal_changed is emitted?
            auto &pt = point.Get();
            pt.button = Point::Button::None;
            pt.action = Point::Action::None;
            // point.signal_changed.Emit();

            signal_released.Emit();

            m_inside_drag = false;
        }
    }
}

