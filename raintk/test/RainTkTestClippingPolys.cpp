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
#include <raintk/RainTkRectangle.hpp>
#include <raintk/RainTkDrawSystem.hpp>

using namespace raintk;

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c(600,600);

    auto draw_system = c.scene->GetDrawSystem();
//    SetupDrawSystem(draw_system);

    draw_system->SetClippingEnabled(false);
    draw_system->SetShowClipOutlines(true);
    draw_system->SetShowBoundingBoxes(false);
    auto root = c.scene->GetRootWidget();

    // Test 1
    // Ensure child widgets completely outside of their
    // parent widgets are clipped properly

    // The child widget should have no clip border around
    // it, indicating that the intersection of the parent
    // and the child widgets had no overlap (thus if
    // clipping was enabled, the child would not show up)

    auto t1_parent = MakeWidget<Rectangle>(root,"t1_parent");
    t1_parent->x = mm(10);
    t1_parent->y = mm(10);
    t1_parent->width = mm(50);
    t1_parent->height = mm(50);
    t1_parent->color = glm::u8vec3(65,65,65);
    t1_parent->clip = true;

    auto t1_child = MakeWidget<Rectangle>(t1_parent,"t1_child");
    t1_child->x = mm(60);
    t1_child->y = mm(10);
    t1_child->z = mm(1);
    t1_child->width = mm(30);
    t1_child->height = mm(30);
    t1_child->color = glm::u8vec3(180,65,65);
    t1_child->rotation = deg(35);


    // Test 2
    // Ensure nested overlapping child widgets are clipped

    // The child widgets should have a clear border around
    // its overlapping area indicating the clip
    auto t2_parent = MakeWidget<Rectangle>(root,"t2_parent");
    t2_parent->x = mm(10);
    t2_parent->y = t1_parent->y.Get()+t1_parent->height.Get()+mm(10);
    t2_parent->width = mm(50);
    t2_parent->height = mm(50);
    t2_parent->color = glm::u8vec3(65,65,65);
    t2_parent->clip = true;

    auto t2_child = MakeWidget<Rectangle>(t2_parent,"t1_child");
    t2_child->x = mm(20);
    t2_child->y = mm(5);
    t2_child->z = mm(1);
    t2_child->width = mm(40);
    t2_child->height = mm(40);
    t2_child->color = glm::u8vec3(180,65,65);
    t2_child->rotation = deg(35);
    t2_child->clip = true;

    auto t2_gc = MakeWidget<Rectangle>(t2_child,"t2_gc");
    t2_gc->y = mm(10);
    t2_gc->z = mm(2);
    t2_gc->width = mm(30);
    t2_gc->height = mm(30);
    t2_gc->color = glm::u8vec3(65,65,180);
    t2_gc->rotation = deg(15);



    c.app->Run();

    return 0;
}
