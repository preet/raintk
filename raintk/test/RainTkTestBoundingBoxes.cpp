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

#include <raintk/test/RainTkTestDrawSystem.hpp>
#include <raintk/RainTkRectangle.hpp>

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;

    auto scene = c.scene.get();

    auto draw_system = scene->GetDrawSystem();
    SetupDrawSystem(draw_system);

    draw_system->SetShowBoundingBoxes(true);
    auto root = scene->GetRootWidget();

    auto widget = MakeWidget<Widget>(scene,root);
    widget->name = "bbox_test";
    widget->width = mm(50);
    widget->height = mm(25);
    widget->x = mm(10);
    widget->y = mm(10);

    auto w2 = MakeWidget<Widget>(scene,widget);
    w2->name = "bbox2";
    w2->x = mm(20);
    w2->y = mm(20);
    w2->width = mm(35);
    w2->height = mm(35);

    c.app->Run();

    return 0;
}
