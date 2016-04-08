/*
   Copyright (C) 2015-2016 Preet Desai (preet.desai@gmail.com)

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

#include <raintk/test/RainTkTestContext.hpp>
#include <raintk/RainTkInputArea.hpp>
#include <raintk/RainTkRectangle.hpp>

namespace raintk
{
    // =========================================================== //

    class InputDelayedCancel : public InputArea
    {
    public:
        using base_type = raintk::InputArea;

        InputDelayedCancel(ks::Object::Key const &key,
                           shared_ptr<Widget> parent,
                           std::string name) :
            InputArea(key,parent,name)
        {}

        void Init(ks::Object::Key const &,
                  shared_ptr<InputDelayedCancel> const &)
        {}

        ~InputDelayedCancel()
        {}

        Property<bool> pressed{
            name+".pressed",false
        };

    private:
        Response handleInput(Point const &pt_global) override
        {
            auto pt_local = TransformPtToLocalCoords(pt_global);
            bool outside = PointOutside(pt_local,this);

            if(!outside)
            {
                if(pressed.Get())
                {
                    if(pt_local.action == Point::Action::Release)
                    {
                        pressed = false;
                        m_list_cancel_history.clear();
                    }
                    m_list_cancel_history.push_back(pt_global);
                }
                else
                {
                    if(pt_local.action == Point::Action::Press)
                    {
                        pressed = true;
                        m_timer =
                                ks::MakeObject<ks::CallbackTimer>(
                                    m_scene->GetEventLoop(),
                                    Milliseconds(200),
                                    [this]{ this->cancelInputsBehindThisInputArea(); });
                        m_timer->SetRepeating(false);
                        m_timer->Start();
                        m_list_cancel_history.push_back(pt_global);
                    }
                }
            }

            // Always pass through
            return Response::Ignore;
        }

        void cancelInput(std::vector<Point> const &) override
        {}

        void cancelInputsBehindThisInputArea()
        {
            rtklog.Trace() << name << ": Canceling inputs behind this InputArea...";

            auto const this_depth =
                    m_cmlist_xf_data->GetComponent(
                        m_entity_id).world_xf[3].z;

            InputArea::cancelInputsBehindDepth(
                        m_scene->GetInputSystem(),
                        m_list_cancel_history,
                        this_depth);

            m_list_cancel_history.clear();
        }

        shared_ptr<ks::CallbackTimer> m_timer;
        std::vector<Point> m_list_cancel_history;
    };

    // =========================================================== //

    class CancelReceiver : public InputArea
    {
    public:
        using base_type = raintk::InputArea;

        CancelReceiver(ks::Object::Key const &key,
                       shared_ptr<Widget> parent,
                       std::string name) :
            InputArea(key,parent,name)
        {}

        void Init(ks::Object::Key const &,
                  shared_ptr<CancelReceiver> const &)
        {}

        ~CancelReceiver()
        {}

    private:
        Response handleInput(Point const &) override
        {
            return Response::Ignore;
        }

        void cancelInput(std::vector<Point> const &list_cancel_pts) override
        {
            for(auto const &cancel_pt : list_cancel_pts)
            {
                Point local_pt = TransformPtToLocalCoords(cancel_pt);
                if(!PointOutside(local_pt,this))
                {
                    rtklog.Trace() << name+": Input canceled!";
                    return;
                }
            }

            rtklog.Trace() << name << ": cancelInput called but point is outside";

        }
    };

    // =========================================================== //
}

using namespace raintk;

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;
    auto root = c.scene->GetRootWidget();

    auto idc_bg = MakeWidget<Rectangle>(root,"red");
    idc_bg->height = mm(50);
    idc_bg->width = mm(50);
    idc_bg->z = mm(10);
    idc_bg->opacity = 0.75;

    // 200ms after clicking on idc, it will tell the
    // InputSystem to cancel all inputs behind it
    // (ie with a smaller z value)
    auto idc = MakeWidget<InputDelayedCancel>(root,"red");
    idc->height = mm(50);
    idc->width  = mm(50);
    idc->z = mm(10);

    auto create_cancel_receiver =
            [&](
            shared_ptr<Widget> parent,
            std::string name,
            glm::u8vec3 color,
            float width,
            float height,
            float x,
            float y,
            float z) {
                    // background
                    auto bg = MakeWidget<Rectangle>(parent,name);
                    bg->height = height;
                    bg->width = width;
                    bg->x = x;
                    bg->y = y;
                    bg->z = z;
                    bg->color = color;

                    // input area
                    auto ia = MakeWidget<CancelReceiver>(parent,name);
                    ia->height = height;
                    ia->width = width;
                    ia->x = x;
                    ia->y = y;
                    ia->z = z;
                };

    // Test 1: Blue is behind but not intersecting
    // along x,y so it should not be canceled
    create_cancel_receiver(
                root,
                "blue",
                glm::u8vec3(0,50,200),
                mm(20),
                mm(20),
                mm(51),
                mm(51),
                mm(9));

    // Test 2: Green is intersecting along x,y but
    // its in front so it should not be canceled
    create_cancel_receiver(
                root,
                "green",
                glm::u8vec3(50,200,0),
                mm(20),
                mm(20),
                mm(45),
                mm(0),
                mm(11));

    // Test 3: Yellow is behind and intersecting along
    // x,y so it should be canceled
    create_cancel_receiver(
                root,
                "yellow",
                glm::u8vec3(250,220,0),
                mm(20),
                mm(20),
                mm(0),
                mm(35),
                mm(8));

    // Run!
    c.app->Run();

    return 0;
}
