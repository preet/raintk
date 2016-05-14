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
#include <raintk/RainTkGrid.hpp>
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

    // Create ImageData
    shared_ptr<ks::ImageData> image_data_ptr;
    {
        ks::Image<ks::RGBA8> img;
        ks::LoadPNG(raintk::test::sun_nasa_png,img);

        // Use the red channel as alpha as well just
        // to test transparency
        for(auto& px : img.GetData())
        {
            px.a = px.r;
        }

        // NOTE:
        // Images with transparency must have a premultiplied alpha.
        // This may need to be done manually depending on source image
        for(auto& px : img.GetData())
        {
            float opacity = px.a/255.0f;
            px.r *= opacity;
            px.g *= opacity;
            px.b *= opacity;
        }

        image_data_ptr.reset(img.ConvertToImageDataPtr().release());
    }


    TestContext c;

    auto root = c.scene->GetRootWidget();
    auto scene = c.scene.get();

    auto grid = MakeWidget<Grid>(scene,root);

    for(uint i=0; i < 3; i++)
    {
        for(uint j=0; j < 3; j++)
        {
            auto image = MakeWidget<Image>(scene,grid);
            image->width = [&](){ return root->width.Get()/3.0f; };
            image->height = [&](){ return root->height.Get()/3.0f; };
            image->source =
                    Image::Source{
                        Image::Source::Lifetime::Permanent,
                        [image_data_ptr](){
                            return image_data_ptr;
                        }
                    };

            image->opacity = (((3*i)+j)*0.1f)+0.1f;
        }
    }

    // Run!
    c.app->Run();

    return 0;
}
