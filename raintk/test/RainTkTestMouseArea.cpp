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

#include <raintk/RainTkRectangle.hpp>
#include <raintk/RainTkMouseArea.hpp>

using namespace raintk;

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;

    auto root =
            c.scene->GetRootWidget();


    auto r0 =
            MakeWidget<Rectangle>(
                c.scene->GetRootWidget(),
                "r0");

    r0->width = mm(60);
    r0->height = mm(60);
    r0->x = 0.25*root->width.Get() - 0.5*r0->width.Get();
    r0->y = 0.5*(root->height.Get() - r0->height.Get());
    r0->z = 5;
    r0->rotation = -0.25;
    r0->color = glm::vec3(60,60,60);

    auto mouse_area0 =
            MakeWidget<MouseArea>(
                r0,"mouse_area0");

    mouse_area0->width = r0->width.Get();
    mouse_area0->height = r0->height.Get();
    mouse_area0->hover = false;

    auto on_pressed0 =
            [&](InputArea::Point::Button) {
                r0->color = glm::vec3(120,120,120);
            };

    auto on_released0 =
            [&](InputArea::Point::Button) {
                r0->color = glm::vec3(60,60,60);
            };

    mouse_area0->signal_pressed.Connect(
                on_pressed0,
                nullptr,
                ks::ConnectionType::Direct);

    mouse_area0->signal_released.Connect(
                on_released0,
                nullptr,
                ks::ConnectionType::Direct);


    auto r1 =
            MakeWidget<Rectangle>(
                c.scene->GetRootWidget(),
                "r1");

    r1->width = mm(60);
    r1->height = mm(60);
    r1->x = 0.75*root->width.Get() - 0.5*r1->width.Get();
    r1->y = 0.5*(root->height.Get() - r1->height.Get());
    r1->z = 2;
    r1->rotation = (-0.25);
    r1->color = glm::vec3(60,60,60);

    auto mouse_area1 =
            MakeWidget<MouseArea>(
                r1,"mouse_area1");

    mouse_area1->width = r1->width.Get();
    mouse_area1->height = r1->height.Get();
    mouse_area1->hover = true;

    bool inside1 = false;

    auto on_entered1 =
            [&]() {
                // Only change colors if a button isn't
                // currently held down
                if(mouse_area1->mouse.Get().button ==
                        InputArea::Point::Button::None)
                {
                    r1->color = glm::vec3(120,120,120);
                }
                inside1 = true;
            };

    auto on_exited1 =
            [&]() {
                // Only change colors if a button isn't
                // currently held down
                if(mouse_area1->mouse.Get().button ==
                        InputArea::Point::Button::None)
                {
                    r1->color = glm::vec3(60,60,60);
                }
                inside1 = false;
            };

    auto on_pressed1 =
            [&](InputArea::Point::Button b) {
                if(b == InputArea::Point::Button::Left)
                {
                    r1->color = glm::vec3(120,200,200);
                }
                else
                {
                    r1->color = glm::vec3(200,120,120);
                }
            };

    auto on_released1 =
            [&](InputArea::Point::Button) {
                if(inside1)
                {
                    r1->color = glm::vec3(120,120,120);
                }
                else
                {
                    r1->color = glm::vec3(60,60,60);
                }
            };

    mouse_area1->signal_entered.Connect(
                on_entered1,
                nullptr,
                ks::ConnectionType::Direct);

    mouse_area1->signal_exited.Connect(
                on_exited1,
                nullptr,
                ks::ConnectionType::Direct);

    mouse_area1->signal_pressed.Connect(
                on_pressed1,
                nullptr,
                ks::ConnectionType::Direct);

    mouse_area1->signal_released.Connect(
                on_released1,
                nullptr,
                ks::ConnectionType::Direct);


    // Run!
    c.app->Run();

    return 0;
}
