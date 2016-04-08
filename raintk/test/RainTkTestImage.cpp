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

namespace raintk
{
    namespace test
    {
        extern std::vector<unsigned char> const sun_nasa_png;
    }
}

using namespace raintk;

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;

    auto root = c.scene->GetRootWidget();

    auto image = MakeWidget<Image>(root,"image");
    image->width = [&](){ return 0.67*root->width.Get(); };
    image->height = [&](){ return 0.67*root->width.Get(); };
    image->x = [&](){ return 0.5*(root->width.Get()-image->width.Get()); };
    image->y = [&](){ return 0.5*(root->height.Get()-image->height.Get()); };

    image->source =
            Image::Source{
                Image::Source::Lifetime::Permanent,
                [](){
                    ks::Image<ks::RGBA8> img;
                    ks::LoadPNG(raintk::test::sun_nasa_png,img);

                    return shared_ptr<ks::ImageData>(
                                img.ConvertToImageDataPtr().release());
                }
            };

    // Run!
    c.app->Run();

    return 0;
}

