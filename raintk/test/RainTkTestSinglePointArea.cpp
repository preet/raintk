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
#include <raintk/RainTkSinglePointArea.hpp>

using namespace raintk;

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;
    auto scene = c.scene.get();
    auto root = c.scene->GetRootWidget();

    // =========================================================== //

    auto r0 =
            MakeWidget<Rectangle>(
                scene,
                c.scene->GetRootWidget());

    float r0_angle_rads = -0.25;

    r0->width = mm(60);
    r0->height = mm(60);
    r0->x = 0.25*root->width.Get() - 0.5*r0->width.Get();
    r0->y = 0.5*(root->height.Get() - r0->height.Get());
    r0->z = 6;
    r0->rotation = r0_angle_rads;
    r0->color = glm::vec3(60,60,60);

    auto sp_area0 =
            MakeWidget<SinglePointArea>(
                scene,
                r0);

    sp_area0->width = mm(60);
    sp_area0->height = mm(60);

    auto on_pressed0 =
            [&](){
                r0->color = glm::vec3(150,60,60);
            };

    auto on_released0 =
            [&](){
                r0->color = glm::vec3(60,60,60);
            };

    auto on_clicked0 =
            [&](){
                r0_angle_rads -= 0.25;
                r0->rotation = r0_angle_rads;
            };

    auto on_point_changed0 =
            [&](){
                auto const &pt = sp_area0->point.Get();
                raintk::rtklog.Trace() << "drag sp0: " << pt.x << "," << pt.y;
            };

    sp_area0->signal_pressed.Connect(
                on_pressed0,
                nullptr,
                ks::ConnectionType::Direct);

    sp_area0->signal_released.Connect(
                on_released0,
                nullptr,
                ks::ConnectionType::Direct);

    sp_area0->signal_clicked.Connect(
                on_clicked0,
                nullptr,
                ks::ConnectionType::Direct);

    sp_area0->point.signal_changed.Connect(
                on_point_changed0,
                nullptr,
                ks::ConnectionType::Direct);

    // =========================================================== //

    auto r1 =
            MakeWidget<Rectangle>(
                scene,
                c.scene->GetRootWidget());

    float r1_angle_rads = -0.35;

    r1->width = mm(60);
    r1->height = mm(60);
    r1->x = r0->x.Get()+mm(60);
    r1->y = 0.5*(root->height.Get() - r1->height.Get());
    r1->z = 4;
    r1->rotation = (r1_angle_rads);
    r1->color = glm::vec3(80,80,80);

    auto sp_area1 =
            MakeWidget<SinglePointArea>(
                scene,r1);

    sp_area1->width = mm(60);
    sp_area1->height = mm(60);

    auto on_pressed1 =
            [&](){
                r1->color = glm::vec3(60,60,150);
            };

    auto on_released1 =
            [&](){
                r1->color = glm::vec3(80,80,80);
            };

    auto on_clicked1 =
            [&](){
                r1_angle_rads -= 0.25;
                r1->rotation = r1_angle_rads;
            };

    auto on_point_changed1 =
            [&](){
                auto const &pt = sp_area1->point.Get();
                raintk::rtklog.Trace() << "drag sp1: " << pt.x << "," << pt.y;
            };

    sp_area1->signal_pressed.Connect(
                on_pressed1,
                nullptr,
                ks::ConnectionType::Direct);

    sp_area1->signal_released.Connect(
                on_released1,
                nullptr,
                ks::ConnectionType::Direct);

    sp_area1->signal_clicked.Connect(
                on_clicked1,
                nullptr,
                ks::ConnectionType::Direct);

    sp_area1->point.signal_changed.Connect(
                on_point_changed1,
                nullptr,
                ks::ConnectionType::Direct);

    // =========================================================== //

    // Run!
    c.app->Run();

    return 0;
}
