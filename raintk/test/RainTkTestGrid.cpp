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
#include <raintk/RainTkGrid.hpp>


std::random_device g_rd;
std::mt19937 g_generator(g_rd());
std::uniform_int_distribution<raintk::uint> g_dis_uint(25,50); // [0,10]

std::vector<ks::shared_ptr<raintk::Rectangle>> list_items;
ks::shared_ptr<raintk::Grid> grid;


int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;

    // colors
    std::vector<glm::u8vec4> list_colors {
        glm::u8vec4{255,0,0,255}, // r
        glm::u8vec4{255,128,0,255}, // o
        glm::u8vec4{255,255,0,255}, // y
        glm::u8vec4{0,255,0,255}, // g
        glm::u8vec4{0,255,255,255}, // c
        glm::u8vec4{128,0,255,255}, // i
        glm::u8vec4{255,0,255,255}, // v
        glm::u8vec4{255,0,127,255} // p
    };

    std::vector<glm::vec2> list_dims {
        glm::vec2{10,10},
        glm::vec2{20,10},
        glm::vec2{10,20},
        glm::vec2{25,10},
        glm::vec2{10,5},
        glm::vec2{10,15},
        glm::vec2{12,10},
        glm::vec2{24,12}
    };

    using raintk::mm;
    auto scene = c.scene.get();
    auto root = c.scene->GetRootWidget();

    grid = raintk::MakeWidget<raintk::Grid>(scene,root);
    grid->row_spacing = mm(2);
    grid->col_spacing = mm(2);
    grid->layout_direction = raintk::Grid::LayoutDirection::TopToBottomRTL;

    for(raintk::uint i=1; i < 9; i++)
    {
        auto rect =
                raintk::MakeWidget<raintk::Rectangle>(
                    scene,
                    grid);

        rect->width = mm(list_dims[i-1].x);
        rect->height = mm(list_dims[i-1].y);
        rect->color = list_colors[i-1];

        list_items.push_back(rect);
    }

    auto rectx =
            raintk::MakeWidget<raintk::Rectangle>(
                scene,
                root);

    raintk::Grid& gridref = *grid;

    rectx->x =
            [&gridref](){
                return (gridref.width.Get() + mm(5));
            };

    auto resize_timer =
            ks::MakeObject<ks::CallbackTimer>(
                c.scene->GetEventLoop(),
                ks::Milliseconds(1000),
                [&](){
                    auto dim_w = list_items[0]->width.Get();
                    auto dim_h = list_items[0]->height.Get();
                    auto dim_c = list_items[0]->color.Get();

                    grid->RemoveChild(list_items[0]);
                    list_items.erase(list_items.begin());

                    auto rect =
                            raintk::MakeWidget<raintk::Rectangle>(
                                scene,
                                grid);

                    rect->width = dim_w;
                    rect->height = dim_h;
                    rect->color = dim_c;

                    list_items.push_back(rect);
                });

    resize_timer->Start();

    // Run!
    c.app->Run();

    // Need to destroy these before the scene is destroyed!
    // (in practice don't use globals or anything that
    //  outlasts the lifetime of the scene)
    list_items.clear();
    grid.reset();

    return 0;
}
