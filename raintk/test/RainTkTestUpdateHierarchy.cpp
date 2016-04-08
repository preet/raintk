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
#include <raintk/RainTkSinglePointArea.hpp>
#include <raintk/RainTkText.hpp>
#include <raintk/RainTkAnimation.hpp>
#include <raintk/RainTkTween.hpp>

using namespace raintk;

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;

    // Add fonts
    auto root = c.scene->GetRootWidget();

    auto rect = MakeWidget<Rectangle>(root,"rect");

    auto words = MakeWidget<Text>(rect,"words");

    rect->width = [&](){ return words->width.Get() + mm(5); };
    rect->height = [&](){ return words->height.Get() + mm(5); };

    words->text = "These are some words!";
    words->font = "FiraSansMinimal.ttf";
    words->color = glm::u8vec3(255,255,255);
    words->x = [&](){ return 0.5f*(rect->width.Get()-words->width.Get()); };
    words->y = [&](){ return 0.5f*(rect->height.Get()-words->height.Get()); };
    words->z = mm(1.0f);

    rtklog.Trace() << "RainTkTestUpdateHierarchy: Widget dimensions before UpdateHierarchy: "
                   << rect->width.Get() << "," << rect->height.Get();

    // UpdateHierarchy should force the text to be laid
    // out and cause @words to update its width and height

    // Confirm that the widget dimensions change accordingly
    rect->UpdateHierarchy();

    rtklog.Trace() << "RainTkTestUpdateHierarchy: Widget dimensions after UpdateHierarchy: "
                   << rect->width.Get() << "," << rect->height.Get();

    // Run!
    c.app->Run();

    return 0;
}
