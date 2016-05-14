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
#include <raintk/RainTkAnimation.hpp>

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


        class SizeAnimation : public raintk::Animation
        {
        public:
            using base_type = raintk::Animation;

            SizeAnimation(ks::Object::Key const &key,
                          Scene* scene) :
                Animation(key,scene)
            {}

            void Init(ks::Object::Key const &,
                      shared_ptr<SizeAnimation> const &)
            {}

            ~SizeAnimation()
            {}

            float from;
            float to;
            float duration;
            Property<float>* property;

        private:
            void start() override
            {
                m_time = 0;
            }

            bool update(float delta_ms) override
            {
                m_time += delta_ms;

                // k is [0,1]
                float k = sin(m_time*0.05f * (k_pi/180.0f)) + 1.0f;
                property->Assign(from + k*(to-from));

                return false;
            }

            void complete() override
            {}

            float m_time;
        };
    }
}

using namespace raintk;

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c(800,480);
    auto scene = c.scene.get();
    auto root = c.scene->GetRootWidget();

    // =========================================================== //

    // When content area is smaller than scroll area

    auto rect_tl = MakeWidget<Rectangle>(scene,root);
    rect_tl->width = root->width.Get()*0.33f;
    rect_tl->height = root->height.Get()*0.5f;
    rect_tl->x = 0.0f;
    rect_tl->y = 0.0f;
    rect_tl->z = 1;
    rect_tl->color = glm::u8vec4(80,60,60,255);


    auto scroll_area_tl =
            MakeWidget<ScrollArea>(
                scene,rect_tl);

    scroll_area_tl->width = rect_tl->width.Get();
    scroll_area_tl->height = rect_tl->height.Get();
    scroll_area_tl->clip = true;
    scroll_area_tl->direction = ScrollArea::Direction::HorizontalAndVertical;
    scroll_area_tl->GetContentParent()->width = scroll_area_tl->width.Get()*0.75;
    scroll_area_tl->GetContentParent()->height = scroll_area_tl->width.Get()*0.75;
    scroll_area_tl->flick = true;


    auto image_tl =
            MakeWidget<Image>(
                scene,scroll_area_tl->GetContentParent());

    image_tl->width = scroll_area_tl->GetContentParent()->width.Get();
    image_tl->height = scroll_area_tl->GetContentParent()->height.Get();
    image_tl->z = 0.1;
    image_tl->smooth = true;
    image_tl->source =
            Image::Source{
                Image::Source::Lifetime::Permanent,
                raintk::test::sun_nasa_png_loader
            };

    // =========================================================== //

    // Animate content area lager than view <---> content area smaller than view
    auto rect_bl = MakeWidget<Rectangle>(scene,root);
    rect_bl->width = root->width.Get()*0.33;
    rect_bl->height = root->height.Get()*0.5;
    rect_bl->x = 0.0f;
    rect_bl->y = rect_bl->height.Get();
    rect_bl->z = 1;
    rect_bl->color = glm::u8vec4(60,80,60,255);

    Property<float> animated_size{scroll_area_tl->width.Get()*0.75f};


    auto scroll_area_bl =
            MakeWidget<ScrollArea>(
                scene,rect_bl);

    scroll_area_bl->width = rect_bl->width.Get();
    scroll_area_bl->height = rect_bl->height.Get();
    scroll_area_bl->clip = true;
    scroll_area_bl->direction = ScrollArea::Direction::HorizontalAndVertical;
    scroll_area_bl->flick = true;
    scroll_area_bl->GetContentParent()->width = [&](){ return animated_size.Get(); };
    scroll_area_bl->GetContentParent()->height = [&](){ return animated_size.Get(); };


    auto size_anim =
            ks::MakeObject<test::SizeAnimation>(
                c.scene.get());

    size_anim->from = scroll_area_tl->width.Get()*0.75;
    size_anim->to = scroll_area_tl->width.Get()*2.75;
    size_anim->duration = 1000;
    size_anim->property = &animated_size;
    size_anim->Start();


    auto image_bl =
            MakeWidget<Image>(
                scene,scroll_area_bl->GetContentParent());

    image_bl->width = [&](){ return scroll_area_bl->GetContentParent()->width.Get(); };
    image_bl->height = [&](){ return scroll_area_bl->GetContentParent()->height.Get(); };
    image_bl->z = 0.1;
    image_bl->smooth = true;
    image_bl->source =
            Image::Source{
            Image::Source::Lifetime::Permanent,
            raintk::test::sun_nasa_png_loader
        };

    // =========================================================== //

    // When content area is larger than scroll area

    auto rect_right = MakeWidget<Rectangle>(scene,root);
    rect_right->width = root->width.Get()*0.66f;
    rect_right->height = root->height.Get();
    rect_right->x = root->width.Get()*0.33f;
    rect_right->y = 0.0f;
    rect_right->z = 1;
    rect_right->color = glm::u8vec4(60,80,60,255);


    auto scroll_area_right =
            MakeWidget<ScrollArea>(
                scene,rect_right);

    scroll_area_right->width = rect_right->width.Get();
    scroll_area_right->height = rect_right->height.Get();
    scroll_area_right->clip = true;
    scroll_area_right->direction = ScrollArea::Direction::HorizontalAndVertical;
    scroll_area_right->GetContentParent()->width = scroll_area_right->width.Get()*4;
    scroll_area_right->GetContentParent()->height = scroll_area_right->width.Get()*4;
    scroll_area_right->flick = true;


    auto image_right =
            MakeWidget<Image>(
                scene,scroll_area_right->GetContentParent());

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
                scene,
                rect_right,
                scroll_area_right.get(),
                scroll_area_right->GetContentParent().get());

    v_scroll_bar->z = 2;


    auto h_scroll_bar =
            MakeWidget<ScrollBar>(
                scene,
                rect_right,
                scroll_area_right.get(),
                scroll_area_right->GetContentParent().get());

    h_scroll_bar->direction=ScrollBar::Direction::Horizontal;
    h_scroll_bar->z = 2;

    // =========================================================== //

    // Run!
    c.app->Run();

    return 0;
}

