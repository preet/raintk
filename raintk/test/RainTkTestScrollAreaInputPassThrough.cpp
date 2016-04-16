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

#include <ks/shared/KsImage.hpp>
#include <raintk/test/RainTkTestContext.hpp>
#include <raintk/RainTkRectangle.hpp>
#include <raintk/RainTkImage.hpp>
#include <raintk/RainTkScrollArea.hpp>
#include <raintk/RainTkText.hpp>

using namespace raintk;

shared_ptr<SinglePointArea> CreateButton(shared_ptr<Widget> parent)
{
    auto scene = parent->GetScene();

    auto button = MakeWidget<SinglePointArea>(scene,parent);
    button->width = mm(30);
    button->height = mm(10);
    button->x = mm(30);
    button->y = mm(30);
    button->z = mm(-2); // must be behind the ScrollArea

    auto bg = MakeWidget<Rectangle>(scene,button);
    bg->name = "bg";
    bg->width = button->width.Get();
    bg->height = button->height.Get();
    bg->color = glm::u8vec3(0,0,0);

    auto text = MakeWidget<Text>(scene,button);
    text->color = glm::u8vec3(255,255,255);
    text->text = "Click Me";
    text->z = mm(2);
    text->font = "FiraSansMinimal.ttf";

    return button;
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;
    auto scene = c.scene.get();
    auto root = c.scene->GetRootWidget();

    auto bg = MakeWidget<Rectangle>(scene,root);
    bg->name = "bg";
    bg->width = mm(90);
    bg->height = mm(90);
    bg->x = 0.5*(root->width.Get()-bg->width.Get());
    bg->y = 0.5*(root->height.Get()-bg->height.Get());
    bg->color = glm::u8vec3(60,60,60);

    auto sa = MakeWidget<ScrollArea>(scene,bg);
    sa->width = bg->width.Get();
    sa->height = bg->height.Get();
    sa->clip = true;
    sa->direction = ScrollArea::Direction::Vertical;
    sa->GetContentParent()->width = sa->width.Get();
    sa->GetContentParent()->height = sa->height.Get()*2.0f;
    sa->flick = true;
    sa->z = mm(10.0f);

    auto image = MakeWidget<Image>(scene,sa->GetContentParent());
    image->width = sa->GetContentParent()->width.Get();
    image->height = sa->GetContentParent()->height.Get();
    image->z = mm(-2.5);
    image->smooth = true;

    auto button = CreateButton(sa->GetContentParent());
    button->signal_pressed.Connect(
                [&](){
                    for(auto& child : button->GetChildren())
                    {
                        if(child->name == "bg")
                        {
                            auto bg = static_cast<Rectangle*>(child.get());
                            bg->color = glm::u8vec3(70,70,70);
                        }
                    }
                });

    button->signal_released.Connect(
                [&](){
                    for(auto& child : button->GetChildren())
                    {
                        if(child->name == "bg")
                        {
                            auto bg = static_cast<Rectangle*>(child.get());
                            bg->color = glm::u8vec3(0,0,0);
                        }
                    }
                });

    button->signal_clicked.Connect(
                [&](){
                    rtklog.Trace() << "Clicked!";
                });

    // Run!
    c.app->Run();

    return 0;
}
