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
#include <raintk/RainTkTextInput.hpp>
#include <raintk/RainTkAlignment.hpp>
#include <raintk/RainTkSinglePointArea.hpp>

// ============================================================= //
// ============================================================= //

using namespace raintk;

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;

    auto scene = c.scene.get();
    auto root = c.scene->GetRootWidget();

    auto text_input = MakeWidget<TextInput>(scene,root);
    text_input->name = "text_input";
    text_input->input_focus = false;
    text_input->width = 300;
    text_input->height = mm(12);
    text_input->x = mm(10);
    text_input->y = mm(10);
    text_input->z = 1.0f;
    text_input->cursor_width = 3;
    text_input->cursor_color = glm::u8vec4(80,80,170,255);
    text_input->GetInputText()->font = "FiraSansMinimal.ttf";
    text_input->GetInputText()->color = glm::u8vec4{250,250,250,255};
    text_input->GetInputText()->size = mm(12);
    text_input->GetInputText()->text = "This is a message to those in command";

    Align::BindCenterToAnchorCenter(text_input.get(),root.get());

    auto rect = MakeWidget<Rectangle>(scene,root);
    rect->width = [&](){ return text_input->width.Get(); };
    rect->height = [&](){ return text_input->height.Get(); };
    rect->x = [&](){ return text_input->x.Get(); };
    rect->y = [&](){ return text_input->y.Get(); };
    rect->z = 0.0f;
    rect->color = glm::u8vec4{0,0,0,255};

    auto r = MakeWidget<Rectangle>(scene,root);
    r->width = rect->width.Get();
    r->height = rect->height.Get();
    r->x = rect->x.Get();
    r->y = rect->y.Get() + rect->height.Get() + mm(10);
    r->color = glm::u8vec4{75,75,75,255};

    auto desc = MakeWidget<Text>(scene,r);
    desc->font = "FiraSansMinimal.ttf";
    desc->color = glm::u8vec4(200,200,255,255);
    desc->text = "Click here to clear focus on TextInput";
    desc->size = mm(4);
    desc->z = 1;

    Align::BindCenterToAnchorCenter(desc.get(),r.get());

    auto spa = MakeWidget<SinglePointArea>(scene,r);
    spa->width = r->width.Get();
    spa->height = r->height.Get();
    spa->z = 1;
    spa->signal_clicked.Connect(
                [&](){
                    text_input->input_focus = false;
                });

    // Run!
    c.app->Run();

    return 0;
}
