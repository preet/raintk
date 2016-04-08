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
#include <raintk/RainTkScrollBar.hpp>

namespace raintk
{
    namespace test
    {
        extern std::vector<unsigned char> const sun_nasa_png;

        auto sun_nasa_png_loader =
                [](){
                    ks::Image<ks::RGBA8> img;
                    ks::LoadPNG(raintk::test::sun_nasa_png,img);

                    return shared_ptr<ks::ImageData>(
                                img.ConvertToImageDataPtr().release());
                };
    }
}

using namespace raintk;

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c(800,480);

    auto root = c.scene->GetRootWidget();

    // =========================================================== //

    // When content area is smaller than scroll area

    auto rect_left = MakeWidget<Rectangle>(root,"rl");
    rect_left->width = root->width.Get()*0.25f;
    rect_left->height = root->height.Get();
    rect_left->x = 0.0f;
    rect_left->y = 0.0f;
    rect_left->z = 1;
    rect_left->color = glm::vec3(80,60,60);


    auto scroll_area_left =
            MakeWidget<ScrollArea>(
                rect_left,"scroll_area_left");

    scroll_area_left->width = rect_left->width.Get();
    scroll_area_left->height = rect_left->height.Get();
    scroll_area_left->clip = true;
    scroll_area_left->direction = ScrollArea::Direction::HorizontalAndVertical;
    scroll_area_left->GetContentParent()->width = scroll_area_left->width.Get()*0.75;
    scroll_area_left->GetContentParent()->height = scroll_area_left->width.Get()*0.75;
    scroll_area_left->flick = true;


    auto image_left =
            MakeWidget<Image>(
                scroll_area_left->GetContentParent(),
                "image_left");

    image_left->width = scroll_area_left->GetContentParent()->width.Get();
    image_left->height = scroll_area_left->GetContentParent()->height.Get();
    image_left->z = 0.1;
    image_left->smooth = true;
    image_left->source =
            Image::Source{
                Image::Source::Lifetime::Permanent,
                raintk::test::sun_nasa_png_loader
            };

    // =========================================================== //

    // When content area is larger than scroll area

    auto rect_right = MakeWidget<Rectangle>(root,"rect_right");
    rect_right->width = root->width.Get()*0.75f;
    rect_right->height = root->height.Get();
    rect_right->x = root->width.Get()*0.25f;
    rect_right->y = 0.0f;
    rect_right->z = 1;
    rect_right->color = glm::vec3(60,80,60);


    auto scroll_area_right =
            MakeWidget<ScrollArea>(
                rect_right,"scroll_area_right");

    scroll_area_right->width = rect_right->width.Get();
    scroll_area_right->height = rect_right->height.Get();
    scroll_area_right->clip = true;
    scroll_area_right->direction = ScrollArea::Direction::HorizontalAndVertical;
    scroll_area_right->GetContentParent()->width = scroll_area_right->width.Get()*4;
    scroll_area_right->GetContentParent()->height = scroll_area_right->width.Get()*4;
    scroll_area_right->flick = true;


    auto image_right =
            MakeWidget<Image>(
                scroll_area_right->GetContentParent(),
                "image_right");

    image_right->width = scroll_area_right->GetContentParent()->width.Get();
    image_right->height = scroll_area_right->GetContentParent()->height.Get();
    image_right->z = 0.1;
    image_right->smooth = true;
    image_right->source =
            Image::Source{
                Image::Source::Lifetime::Permanent,
                raintk::test::sun_nasa_png_loader
            };

    // Add some scroll bars
    auto v_scroll_bar =
            MakeWidget<ScrollBar>(
                rect_right,
                "",
                scroll_area_right.get(),
                scroll_area_right->GetContentParent().get());

    v_scroll_bar->z = 2;


    auto h_scroll_bar =
            MakeWidget<ScrollBar>(
                rect_right,
                "",
                scroll_area_right.get(),
                scroll_area_right->GetContentParent().get());

    h_scroll_bar->direction=ScrollBar::Direction::Horizontal;
    h_scroll_bar->z = 2;

    // =========================================================== //

    // Run!
    c.app->Run();

    return 0;
}
