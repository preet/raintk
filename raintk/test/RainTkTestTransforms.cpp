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

using namespace raintk;

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;
    auto scene = c.scene.get();
    auto parent = c.scene->GetRootWidget();

    // Transforms should accumulate up from
    // widget parents

    // As a simple test we see if the z position
    // of the parent is reflected in the transform
    // of a child rectangle

    // The blue rectangle should be drawn above
    // the red one

    auto upper_plane =MakeWidget<Widget>(scene,parent);
    upper_plane->z = mm(5);


    auto lower_plane =MakeWidget<Widget>(scene,parent);
    lower_plane->z = mm(1);


    auto red = MakeWidget<Rectangle>(scene,lower_plane);
    red->x = (parent->width.Get() - red->width.Get())*0.5;
    red->height = parent->height.Get();
    red->color = glm::vec4{255,0,0,255};


    auto blue = MakeWidget<Rectangle>(scene,upper_plane);
    blue->y = (parent->height.Get() - blue->height.Get())*0.5;
    blue->width = parent->width.Get();
    blue->color = glm::vec4{0,0,255,255};


    // Run!
    c.app->Run();

    return 0;
}
