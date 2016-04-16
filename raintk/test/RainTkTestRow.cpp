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

#include <random>
#include <ks/shared/KsCallbackTimer.hpp>
#include <raintk/RainTkRectangle.hpp>
#include <raintk/RainTkRow.hpp>
#include <raintk/RainTkColumn.hpp>
#include <raintk/RainTkGrid.hpp>

std::vector<ks::shared_ptr<raintk::Rectangle>> g_list_items;

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;

    auto scene = c.scene.get();

    // colors
    std::vector<glm::u8vec3> list_colors {
        glm::u8vec3{255,0,0}, // r
        glm::u8vec3{255,128,0}, // o
        glm::u8vec3{255,255,0}, // y
        glm::u8vec3{0,255,0}, // g
        glm::u8vec3{0,255,255}, // c
        glm::u8vec3{128,0,255}, // i
        glm::u8vec3{255,0,255}, // v
        glm::u8vec3{255,0,127} // p
    };

    std::vector<glm::vec2> list_dims {
        glm::vec2{10,10},
        glm::vec2{11,10},
        glm::vec2{12,10},
        glm::vec2{13,10},
        glm::vec2{14,10},
        glm::vec2{15,10},
        glm::vec2{16,10},
        glm::vec2{17,10}
    };

    using raintk::mm;


    // Test simple LTR
    auto row_ltr =
            raintk::MakeWidget<raintk::Row>(
                scene,
                scene->GetRootWidget());

    row_ltr->x = mm(2);
    row_ltr->y = mm(2);
    row_ltr->spacing = mm(2);
    row_ltr->layout_direction =
            raintk::Row::LayoutDirection::LeftToRight;

    for(raintk::uint i=1; i < 9; i++)
    {
        auto rect =
                raintk::MakeWidget<raintk::Rectangle>(
                    scene,
                    row_ltr);

        rect->width = mm(list_dims[i-1].x);
        rect->height = mm(list_dims[i-1].y);
        rect->color = list_colors[i-1];
    }


    // Test simple RTL
    auto row_rtl =
            raintk::MakeWidget<raintk::Row>(
                scene,
                c.scene->GetRootWidget());

    row_rtl->x = mm(2);
    row_rtl->y = mm(2) + mm(2) + mm(10);
    row_rtl->spacing = mm(2);
    row_rtl->layout_direction =
            raintk::Row::LayoutDirection::RightToLeft;

    for(raintk::uint i=1; i < 9; i++)
    {
        auto rect =
                raintk::MakeWidget<raintk::Rectangle>(
                    scene,
                    row_rtl);

        rect->width = mm(list_dims[i-1].x);
        rect->height = mm(list_dims[i-1].y);
        rect->color = list_colors[i-1];
    }


    // Test changing row item sizes
    auto row_upd =
            raintk::MakeWidget<raintk::Row>(
                scene,
                c.scene->GetRootWidget());

    row_upd->x = mm(2);
    row_upd->y = mm(2+2+2+10+10);
    row_upd->spacing = mm(2);

    std::vector<raintk::Rectangle*> list_row_items;

    for(raintk::uint i=1; i < 9; i++)
    {
        auto rect =
                raintk::MakeWidget<raintk::Rectangle>(
                    scene,
                    row_upd);

        rect->width = mm(10);
        rect->height = mm(list_dims[i-1].y);
        rect->color = list_colors[i-1];

        list_row_items.push_back(rect.get());
    }

    float slice = 6.28/200.0f;

    // Periodically change the width of the
    // Row items to see if the layout is updated
    ks::shared_ptr<ks::CallbackTimer> resize_timer =
            ks::MakeObject<ks::CallbackTimer>(
                c.scene->GetEventLoop(),
                ks::Milliseconds(33),
                [&]()
                {
                    slice += 6.28/200.0f;

                    uint i=0;
                    for(auto widget : list_row_items)
                    {
                        if(i%2==0)
                        {
                            widget->width = mm(10) + mm((sin(slice)*5));
                        }
                        else
                        {
                            widget->width = mm(10) + mm((cos(slice)*5));
                        }
                        i++;
                    }
                });

    resize_timer->Start();


    // Run!
    c.app->Run();


    return 0;
}
