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

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;

    auto& root = *(c.scene->GetRootWidget());

    // colors
    std::vector<glm::u8vec4> list_colors {
        glm::u8vec4{255,0,0,255},
        glm::u8vec4{255,128,0,255},
        glm::u8vec4{255,255,0,255},
        glm::u8vec4{0,255,0,255},
        glm::u8vec4{0,0,255,255},
        glm::u8vec4{128,0,255,255},
        glm::u8vec4{255,0,0,255},
        glm::u8vec4{255,128,0,255},
        glm::u8vec4{255,255,0,255},
        glm::u8vec4{0,255,0,255},
        glm::u8vec4{0,0,255,255},
        glm::u8vec4{128,0,255,255}
    };

    for(int i=0; i < 12; i++)
    {
        auto& xpr_rect =
                *(raintk::MakeWidget<raintk::Rectangle>(
                      c.scene.get(),
                      c.scene->GetRootWidget()));

        xpr_rect.width =
                [&](){
                    return (0.167f*root.width.Get());
                };

        xpr_rect.height =
                [&](){
                    return xpr_rect.width.Get();
                };

        xpr_rect.x =
                [&,i](){
                    return (0.5*i*xpr_rect.width.Get());
                };

        xpr_rect.y =
                [&](){
                    return (0.25*root.height.Get() - 0.5*xpr_rect.height.Get());
                };

        xpr_rect.z = 20-i;


        xpr_rect.rotation = raintk::deg(7.5*i);

        xpr_rect.color = list_colors[i];

        xpr_rect.opacity = 0.5f;
    }

    for(int i=0; i < 12; i++)
    {
        auto& opq_rect =
                *(raintk::MakeWidget<raintk::Rectangle>(
                    c.scene.get(),
                    c.scene->GetRootWidget()));

        opq_rect.width =
                [&](){
                    return (0.167f*root.width.Get());
                };

        opq_rect.height =
                [&](){
                    return opq_rect.width.Get();
                };

        opq_rect.x =
                [&,i](){
                    return (0.5*i*opq_rect.width.Get());
                };

        opq_rect.y =
                [&](){
                    return (0.5*root.height.Get() - 0.5*opq_rect.height.Get());
                };

        opq_rect.z = 20-i;


        opq_rect.rotation = raintk::deg(7.5*i);

        opq_rect.color = list_colors[i];

        opq_rect.opacity = 1.0f;
    }

    // Run!
    c.app->Run();

    return 0;
}
