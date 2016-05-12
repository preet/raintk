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
#include <raintk/RainTkImageAtlas.hpp>
#include <raintk/RainTkAtlasImage.hpp>

using namespace raintk;

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;
    auto scene = c.scene.get();
    auto root = c.scene->GetRootWidget();

    auto image_atlas_ptr =
            make_shared<ImageAtlas>(
                scene,8,8,2,2,ks::gl::Texture2D::Format::RGB8);

    ImageAtlas& image_atlas = *image_atlas_ptr;

    for(uint i=0; i < 4; i++)
    {
        ImageAtlas::Source image_source{
            ImageAtlas::Source::Lifetime::Permanent,
            [i](){
                std::vector<ks::RGB8> list_colors{
                    ks::RGB8{255,0,0},
                    ks::RGB8{0,255,0},
                    ks::RGB8{0,0,255},
                    ks::RGB8{255,255,0}
                };

                ks::Image<ks::RGB8> image(2,2);
                for(uint j=0; j < 4; j++)
                {
                    image.GetData().push_back(list_colors[i]);
                }
                return shared_ptr<ks::ImageData>(
                            image.ConvertToImageDataPtr().release());
            }
        };

        image_atlas.AddImage(image_source);
    }

    auto atlas_image0 = MakeWidget<AtlasImage>(scene,root,image_atlas_ptr,1);
    atlas_image0->width = 100;
    atlas_image0->height = 100;

    auto atlas_image1 = MakeWidget<AtlasImage>(scene,root,image_atlas_ptr,2);
    atlas_image1->width = 100;
    atlas_image1->height = 100;
    atlas_image1->x = 100;

    auto atlas_image2 = MakeWidget<AtlasImage>(scene,root,image_atlas_ptr,3);
    atlas_image2->width = 100;
    atlas_image2->height = 100;
    atlas_image2->y = 100;

    auto atlas_image3 = MakeWidget<AtlasImage>(scene,root,image_atlas_ptr,4);
    atlas_image3->width = 100;
    atlas_image3->height = 100;
    atlas_image3->x = 100;
    atlas_image3->y = 100;


    // Run!
    c.app->Run();

    return 0;
}

