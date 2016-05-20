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
#include <raintk/RainTkColumn.hpp>
#include <raintk/RainTkText.hpp>

#include <ks/shared/KsCallbackTimer.hpp>

// ============================================================= //
// ============================================================= //

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;

    // Set colors
    glm::u8vec4 color_bg{25,25,25,255};
    glm::u8vec4 color_fg{228,228,228,255};

    auto scene = c.scene.get();

    auto root = c.scene->GetRootWidget();

    auto rect =
            raintk::MakeWidget<raintk::Rectangle>(
                scene,root);

    rect->width = root->width.Get();
    rect->height = root->height.Get();
    rect->color = color_bg;

    auto divider =
            raintk::MakeWidget<raintk::Rectangle>(
                scene,root);

    divider->width = raintk::mm(0.5);
    divider->height = root->height.Get();
    divider->x = root->width.Get()*0.5 - raintk::mm(0.25);
    divider->z = raintk::mm(1.0);
    divider->color = color_fg;

    // Left Column
    auto left_column =
            raintk::MakeWidget<raintk::Column>(
                scene,root);

    left_column->y = raintk::mm(2.0);
    left_column->spacing = raintk::mm(2.5);

    auto make_text =
            [&]() -> ks::shared_ptr<raintk::Text>
            {
                auto t = raintk::MakeWidget<raintk::Text>(scene,left_column);
                t->font = "FiraSansMinimal.ttf";
                t->color = color_fg;
                t->x = raintk::mm(2);
                t->z = raintk::mm(1);
                t->size = raintk::mm(4);
                t->height_calc = raintk::Text::HeightCalc::FontBounds;

                return t;
            };

    // Empty text
    {
        make_text();
    }

    // Single line
    {
        auto label = make_text();
        label->text = "This text shows a single line";
    }

    // Line breaks
    {
        auto label = make_text();

        label->text =
                "This text shows multiple lines\n"
                "using control characters\n"
                "like LF to break";
    }

    // Line width
    {
        auto label = make_text();

        label->text =
                "This text shows multiple lines that break "
                "automatically when a maximum line width is "
                "specified";

        label->line_width = divider->x.Get() - raintk::mm(3.0);
    }

    // Alignment
    {
        auto label = make_text();
        label->text = "This text shows\n center alignment,";
        label->alignment = raintk::Text::Alignment::Center;

        label = make_text();
        label->text = "and this text shows\n right alignment";
        label->alignment = raintk::Text::Alignment::Right;
    }


    // Right Column
    auto right_column =
            raintk::MakeWidget<raintk::Column>(
                scene,root);

    right_column->y = raintk::mm(2.0);

    // Dynamically create and destroy a bunch of text to ensure
    // batch creation and destruction is working propertly
    std::vector<ks::shared_ptr<raintk::Text>> list_right_column_text;
    std::vector<ks::uint> list_text_item_count{0,10,20,30};
    ks::uint text_item_count_idx=0;

    auto make_smaller_text =
            [&]() -> ks::shared_ptr<raintk::Text>
            {
                auto t = raintk::MakeWidget<raintk::Text>(scene,right_column);
                t->font = "FiraSansMinimal.ttf";
                t->color = color_fg;
                t->x = raintk::mm(2) + 300;
                t->z = raintk::mm(1);
                t->size = raintk::mm(2.5);
                t->height_calc = raintk::Text::HeightCalc::FontBounds;

                return t;
            };

    auto text_timer =
            ks::MakeObject<ks::CallbackTimer>(
                c.scene->GetEventLoop(),
                raintk::Milliseconds(1000),
                [&]()
                {
                    for(auto& label : list_right_column_text)
                    {
                        label->GetParent()->RemoveChild(label);
                    }
                    list_right_column_text.clear();

                    uint index = text_item_count_idx%4;
                    for(ks::uint i=0; i < list_text_item_count[index]; i++)
                    {
                        auto label = make_smaller_text();
                        label->text = ks::ToString(i)+". A Quick Brown Fox Jumps Over The Lazy Dog 0123456789";
                        label->opacity = std::min(1.0f,i*0.05f + 0.1f);
                        list_right_column_text.push_back(label);
                    }

                    text_item_count_idx++;
                });

    text_timer->Start();


    // Run!
    c.app->Run();

    return 0;
}
