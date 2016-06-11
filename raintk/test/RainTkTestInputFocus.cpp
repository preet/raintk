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

#include <raintk/RainTkSinglePointArea.hpp>
#include <raintk/RainTkRectangle.hpp>
#include <raintk/RainTkText.hpp>
#include <raintk/RainTkRow.hpp>
#include <raintk/RainTkColumn.hpp>

using namespace raintk;

namespace
{
    struct Label
    {
        shared_ptr<Rectangle> rect;
        shared_ptr<Text> text;
    };

    struct Button
    {
        Label label;
        shared_ptr<SinglePointArea> spa;
    };

    auto color_white = glm::u8vec4{255,255,255,255};
    auto color_black = glm::u8vec4{0,0,0,255};
    auto color_blue = glm::u8vec4{0,55,100,255};

    Label CreateLabel(Scene* scene, shared_ptr<Widget> parent)
    {
        Label label;

        label.rect = MakeWidget<Rectangle>(scene,parent);
        label.rect->color = color_black;

        label.text = MakeWidget<Text>(scene,label.rect);
        label.text->height_calc = Text::HeightCalc::FontBounds;
        label.text->font = "FiraSansMinimal.ttf";
        label.text->color = color_white;
        label.text->size = mm(8);
        label.text->z = 0.1f;

        auto t = label.text.get();

        label.rect->width = [t](){ return t->width.Get(); };
        label.rect->height = [t](){ return t->height.Get(); };

        label.text->text = [t](){
            auto f = t->input_focus.Get();
            if(f)
            {
                return "Focus: True";
            }
            else
            {
                return "Focus: False";
            }
        };

        return label;
    }

    Button CreateButton(Scene* scene, shared_ptr<Widget> parent, std::string msg)
    {
        Button button;
        button.label = CreateLabel(scene,parent);
        button.label.rect->color = color_blue;
        button.label.text->text = msg;
        button.spa = MakeWidget<SinglePointArea>(scene,button.label.rect);

        auto t = button.label.text.get();

        button.spa->width = [t](){ return t->width.Get(); };
        button.spa->height = [t](){ return t->height.Get(); };

        return button;
    }
}




int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;

    auto scene = c.scene.get();
    auto root = c.scene->GetRootWidget();

    auto col = MakeWidget<Column>(scene,root);
    col->spacing = mm(5);
    col->y = mm(5);

    std::vector<Label> labels;
    std::vector<Button> set_buttons;
    std::vector<Button> clear_buttons;

    for(uint i=0; i < 4; i++)
    {
        auto row = MakeWidget<Row>(scene,col);
        row->spacing = mm(5);

        labels.push_back(CreateLabel(scene,row));
        set_buttons.push_back(CreateButton(scene,row,"Set"));
        clear_buttons.push_back(CreateButton(scene,row,"Clear"));

        auto text_ptr = labels.back().text.get();

        set_buttons.back().spa->signal_clicked.Connect(
                    [text_ptr](){
                        text_ptr->input_focus = true;
                    });

        clear_buttons.back().spa->signal_clicked.Connect(
                    [text_ptr](){
                        text_ptr->input_focus = false;
                    });
    }

    // Run!
    c.app->Run();

    return 0;
}
