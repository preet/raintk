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

#include <raintk/test/RainTkTestContext.hpp>
#include <raintk/RainTkWidget.hpp>
#include <raintk/RainTkAlignment.hpp>
#include <raintk/RainTkTransformSystem.hpp>

#include <cassert>

using namespace raintk;

float GetWorldHCenter(shared_ptr<Widget> const &w)
{
    return Widget::CalcWorldCoords(
                w.get(),
                glm::vec2(
                    0.5f*(w->width.Get()) + w->x.Get(),
                    0.0f)).x;
}

float GetWorldVCenter(shared_ptr<Widget> const &w)
{
    return Widget::CalcWorldCoords(
                w.get(),
                glm::vec2(
                    0.0f,
                    0.5f*(w->height.Get()) + w->y.Get())).y;
}

float GetLeft(shared_ptr<Widget> const &w)
{
    return Widget::CalcWorldCoords(
                w.get(),
                glm::vec2(
                    w->x.Get(),
                    0.0f)).x;
}

float GetRight(shared_ptr<Widget> const &w)
{
    return Widget::CalcWorldCoords(
                w.get(),
                glm::vec2(
                    w->x.Get() + w->width.Get(),
                    0.0f)).x;
}

float GetTop(shared_ptr<Widget> const &w)
{
    return Widget::CalcWorldCoords(
                w.get(),
                glm::vec2(
                    0.0f,
                    w->y.Get())).y;
}

float GetBottom(shared_ptr<Widget> const &w)
{
    return Widget::CalcWorldCoords(
                w.get(),
                glm::vec2(
                    0.0f,
                    w->y.Get() + w->height.Get())).y;
}

void TestAssignment(shared_ptr<Widget> &widget,
                    shared_ptr<Widget> &anchor)
{
    anchor->x = 10;
    anchor->y = 20;
    anchor->width = 30;
    anchor->height = 40;

    widget->width = 20;
    widget->height = 30;

    // We need to call update to have the correct
    // transform matrices set
    ks::TimePoint t0,t1;
    widget->GetScene()->GetTransformSystem()->Update(t0,t1);

    Align::AssignCenterToAnchorCenter(widget,anchor);
    assert(GetWorldHCenter(widget)==25);
    assert(GetWorldVCenter(widget)==40);


    Align::AssignHCenterToAnchorHCenter(widget,anchor);
    assert(GetWorldHCenter(widget)==25);

    Align::AssignRightToAnchorLeft(widget,anchor);
    assert(GetRight(widget)==10);

    Align::AssignLeftToAnchorLeft(widget,anchor);
    assert(GetLeft(widget)==10);

    Align::AssignLeftToAnchorRight(widget,anchor);
    assert(GetLeft(widget)==40);

    Align::AssignRightToAnchorRight(widget,anchor);
    assert(GetRight(widget)==40);


    Align::AssignVCenterToAnchorVCenter(widget,anchor);
    assert(GetWorldVCenter(widget)==40);

    Align::AssignBottomToAnchorTop(widget,anchor);
    assert(GetBottom(widget)==20);

    Align::AssignTopToAnchorTop(widget,anchor);
    assert(GetTop(widget)==20);

    Align::AssignBottomToAnchorBottom(widget,anchor);
    assert(GetBottom(widget)==60);

    Align::AssignTopToAnchorBottom(widget,anchor);
    assert(GetTop(widget)==60);
}

void TestBinding(shared_ptr<Widget> &widget,
                 shared_ptr<Widget> &anchor)
{
    anchor->x = 10;
    anchor->y = 20;
    anchor->width = 30;
    anchor->height = 40;

    widget->width = 20;
    widget->height = 30;

    // We need to call update to have the correct
    // transform matrices set
    ks::TimePoint t0,t1;
    widget->GetScene()->GetTransformSystem()->Update(t0,t1);

    Align::BindCenterToAnchorCenter(widget,anchor);
    assert(GetWorldHCenter(widget)==25);
    assert(GetWorldVCenter(widget)==40);


    Align::BindHCenterToAnchorHCenter(widget,anchor);
    assert(GetWorldHCenter(widget)==25);

    Align::BindRightToAnchorLeft(widget,anchor);
    assert(GetRight(widget)==10);

    Align::BindLeftToAnchorLeft(widget,anchor);
    assert(GetLeft(widget)==10);

    Align::BindLeftToAnchorRight(widget,anchor);
    assert(GetLeft(widget)==40);

    Align::BindRightToAnchorRight(widget,anchor);
    assert(GetRight(widget)==40);


    Align::BindVCenterToAnchorVCenter(widget,anchor);
    assert(GetWorldVCenter(widget)==40);

    Align::BindBottomToAnchorTop(widget,anchor);
    assert(GetBottom(widget)==20);

    Align::BindTopToAnchorTop(widget,anchor);
    assert(GetTop(widget)==20);

    Align::BindBottomToAnchorBottom(widget,anchor);
    assert(GetBottom(widget)==60);

    Align::BindTopToAnchorBottom(widget,anchor);
    assert(GetTop(widget)==60);
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;

    auto scene = c.scene.get();
    auto root = c.scene->GetRootWidget();

    {
        // Test sibling assignment
        auto anchor = MakeWidget<Widget>(scene,root);
        auto widget = MakeWidget<Widget>(scene,root);
        TestAssignment(widget,anchor);
    }

    {
        // Test parent assignment
        auto parent = MakeWidget<Widget>(scene,root);
        auto widget = MakeWidget<Widget>(scene,parent);
        TestAssignment(widget,parent);
    }

    {
        // Test sibling binding
        auto anchor = MakeWidget<Widget>(scene,root);
        auto widget = MakeWidget<Widget>(scene,root);
        TestBinding(widget,anchor);
    }

    {
        // Test parent binding
        auto parent = MakeWidget<Widget>(scene,root);
        auto widget = MakeWidget<Widget>(scene,parent);
        TestBinding(widget,parent);
    }




    // Run!
    c.app->Run();

    return 0;
}
