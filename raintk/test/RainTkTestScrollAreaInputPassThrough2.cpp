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
#include <raintk/RainTkScrollArea.hpp>
#include <raintk/RainTkText.hpp>
#include <raintk/RainTkInputSystem.hpp>

#include <cmath>

using namespace raintk;

void CreateButtonColumn(shared_ptr<Widget> p)
{
    float spacing = mm(2.0f);
    float button_height = mm(30.0f);
    float button_width = p->width.Get()-2*spacing;

    auto scene = p->GetScene();
    auto &column = p;
    column->height = (button_height+spacing)*10;

    float button_y = 0.0f;

    for(uint i=0; i < 10; i++)
    {
        auto button = MakeWidget<Widget>(scene,column);
        button->width = button_width;
        button->height = button_height;
        button->x = spacing;
        button->y = button_y;

        auto spa = MakeWidget<SinglePointArea>(scene,button);
        spa->width = button->width.Get();
        spa->height = button->height.Get();
        spa->z = mm(-1.0f);

        auto bg = MakeWidget<Rectangle>(scene,button);
        bg->width = button->width.Get();
        bg->height = button->height.Get();
        bg->color = glm::u8vec4(0,0,0,255);
        bg->z = mm(3.0f);

        auto text = MakeWidget<Text>(scene,bg);
        text->color = glm::u8vec4(255,255,255,255);
        text->text = "Click Me";
        text->z = mm(0.5f);
        text->font = "FiraSansMinimal.ttf";

        spa->signal_clicked.Connect(
                    [&](){
                        rtklog.Trace() << "Clicked!";
                    });

        button_y += (spacing + button_height);
    }
}

void CreateVerticalScrollAreas(shared_ptr<Widget> p)
{
    auto scene = p->GetScene();

    float vsa_width = p->width.Get()/3.0f;
    float vsa_height = p->height.Get();
    float vsa_margin = mm(15.0f);
    float vsa_x = 0.0f;

    for(uint i=0; i < 3; i++)
    {
        // list view
        auto base = MakeWidget<Widget>(scene,p);
        base->width = vsa_width;
        base->height = vsa_height;
        base->x = vsa_x;

        auto bg = MakeWidget<Rectangle>(scene,base);
        bg->width = vsa_width - 2*vsa_margin;
        bg->height = vsa_height - 2*vsa_margin;
        bg->x = vsa_margin;
        bg->y = vsa_margin;
        bg->z = mm(1.0f);

        auto vsa = MakeWidget<ScrollArea>(scene,base);
        vsa->width = bg->width.Get();
        vsa->height = bg->height.Get();
        vsa->x = vsa_margin;
        vsa->y = vsa_margin;
        vsa->direction = ScrollArea::Direction::Vertical;
        vsa->flick = true;
        vsa->z = mm(-1.0f);
        vsa->GetContentParent()->width = vsa->width.Get();
        vsa->GetContentParent()->height = vsa->height.Get();
        vsa->clip = true;

        CreateButtonColumn(vsa->GetContentParent());

        vsa_x += vsa_width;
    }
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c(600,600);
    auto scene = c.scene.get();
    auto root = c.scene->GetRootWidget();

    // horizontal scroll area
    auto hsa = MakeWidget<ScrollArea>(scene,root);
    hsa->width = root->width.Get();
    hsa->height = root->height.Get();
    hsa->clip = true;
    hsa->direction = ScrollArea::Direction::Horizontal;
    hsa->GetContentParent()->width = hsa->width.Get()*3.0f;
    hsa->GetContentParent()->height = hsa->height.Get();
    hsa->flick = true;

    // vertical scroll areas
    CreateVerticalScrollAreas(hsa->GetContentParent());

    // Run!
    c.app->Run();

    return 0;
}
