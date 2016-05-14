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
                           Scene* scene,
                           shared_ptr<Widget> parent) :
            InputArea(key,scene,parent)
        {}

        void Init(ks::Object::Key const &,
                  shared_ptr<InputDelayedCancel> const &)
        {}

        ~InputDelayedCancel()
        {}

        Property<bool> pressed{
            false
        };

    private:
        Response handleInput(Point const &pt_local,bool inside) override
        {
            bool outside = !inside;

            if(!outside)
            {
                if(pressed.Get())
                {
                    if(pt_local.action == Point::Action::Release)
                    {
                        pressed = false;
                        m_list_cancel_history.clear();
                    }
                    m_list_cancel_history.push_back(pt_local);
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
                        m_list_cancel_history.push_back(pt_local);
                    }
                }
            }

            // Always pass through
            return Response::Ignore;
        }

        void cancelInput() override
        {}

        void cancelInputsBehindThisInputArea()
        {
            rtklog.Trace() << name << ": Canceling inputs behind this InputArea...";

            this->cancelInputsBehindWidget(
                        m_list_cancel_history);

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
                       Scene* scene,
                       shared_ptr<Widget> parent) :
            InputArea(key,scene,parent)
        {}

        void Init(ks::Object::Key const &,
                  shared_ptr<CancelReceiver> const &)
        {}

        ~CancelReceiver()
        {}

    private:
        Response handleInput(Point const &,bool) override
        {
            return Response::Ignore;
        }

        void cancelInput() override
        {
            rtklog.Trace() << name+": Input canceled!";
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
    auto scene = c.scene.get();
    auto root = c.scene->GetRootWidget();

    auto idc_bg = MakeWidget<Rectangle>(scene,root);
    idc_bg->height = mm(50);
    idc_bg->width = mm(50);
    idc_bg->z = mm(10);
    idc_bg->opacity = 0.75;

    // 200ms after clicking on idc, it will tell the
    // InputSystem to cancel all inputs behind it
    // (ie with a smaller z value)
    auto idc = MakeWidget<InputDelayedCancel>(scene,root);
    idc->name = "red";
    idc->height = mm(50);
    idc->width  = mm(50);
    idc->z = mm(10);

    auto create_cancel_receiver =
            [&](
            shared_ptr<Widget> parent,
            std::string name,
            glm::u8vec4 color,
            float width,
            float height,
            float x,
            float y,
            float z) {
                    // background
                    auto bg = MakeWidget<Rectangle>(scene,parent);
                    bg->name = name;
                    bg->height = height;
                    bg->width = width;
                    bg->x = x;
                    bg->y = y;
                    bg->z = z;
                    bg->color = color;

                    // input area
                    auto ia = MakeWidget<CancelReceiver>(scene,parent);
                    ia->name = name;
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
                glm::u8vec4(0,50,200,255),
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
                glm::u8vec4(50,200,0,255),
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
                glm::u8vec4(250,220,0,255),
                mm(20),
                mm(20),
                mm(0),
                mm(35),
                mm(8));

    // Run!
    c.app->Run();

    return 0;
}
