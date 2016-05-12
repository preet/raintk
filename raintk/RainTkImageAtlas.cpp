/*
   Copyright (C) 2016 Preet Desai (preet.desai@gmail.com)

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
#include <raintk/RainTkImageAtlas.hpp>
#include <raintk/RainTkScene.hpp>
#include <raintk/RainTkDrawSystem.hpp>

namespace raintk
{
    // ============================================================= //
    // ============================================================= //

    ImageAtlasNoSpaceAvail::ImageAtlasNoSpaceAvail() :
        ks::Exception(ks::Exception::ErrorLevel::ERROR,"")
    {}

    ImageAtlasIdNotFound::ImageAtlasIdNotFound() :
        ks::Exception(ks::Exception::ErrorLevel::ERROR,"")
    {}

    // ============================================================= //
    // ============================================================= //

    ImageAtlas::ImageAtlas(Scene* scene,
                           uint width_px,
                           uint height_px,
                           uint x_regions,
                           uint y_regions,
                           ks::gl::Texture2D::Format format) :
        m_scene(scene),
        m_width_px(width_px),
        m_height_px(height_px),
        m_x_regions(x_regions),
        m_y_regions(y_regions),
        m_format(format)
    {
        // This object represents a single texture
        // with N regions
        uint const region_width = m_width_px/m_x_regions;
        uint const region_height = m_height_px/m_y_regions;
        uint const region_count = x_regions*y_regions;
        for(uint i=0; i < region_count; i++)
        {
            uint const x = (i%2)*region_width;
            uint const y = (i/2)*region_height;

            m_list_regions.emplace_back();
            m_list_regions.back().x = x;
            m_list_regions.back().y = y;
            m_list_regions.back().bin_packer =
                    make_unique<ks::BinPackShelf>(
                        region_width,
                        region_height,
                        1);
        }

        // Create Texture and TextureSet
        auto texture = make_shared<ks::gl::Texture2D>(m_format);

        texture->SetFilterModes(
                    ks::gl::Texture2D::Filter::Nearest,
                    ks::gl::Texture2D::Filter::Nearest);

        texture->SetWrapModes(
                    ks::gl::Texture::Wrap::ClampToEdge,
                    ks::gl::Texture::Wrap::ClampToEdge);

        ks::Image<ks::RGB8> blank_image(
                    m_width_px,
                    m_height_px,
                    ks::RGB8{0,0,0});

        texture->UpdateTexture(
                    ks::gl::Texture2D::Update{
                        ks::gl::Texture2D::Update::ReUpload,
                        glm::u16vec2(0,0),
                        shared_ptr<ks::ImageData>(
                        blank_image.ConvertToImageDataPtr().release()
                        )
                    });

        m_texture_set = make_shared<ks::draw::TextureSet>();

        m_texture_set->list_texture_desc.emplace_back(
                    std::move(texture),0); // 0 = tex unit

        m_texture_set_id =
                m_scene->GetDrawSystem()->
                RegisterTextureSet(m_texture_set);
    }

    ImageAtlas::~ImageAtlas()
    {
        m_scene->GetDrawSystem()->
                RemoveTextureSet(m_texture_set_id);
    }

    Id ImageAtlas::AddImage(Source source)
    {
        Id new_entry_id;

        // Try to create an Entry
        shared_ptr<ks::ImageData> image_data = source.load();

        for(auto& region : m_list_regions)
        {
            ks::BinPackRectangle bin_rect;
            bin_rect.width = image_data->width;
            bin_rect.height = image_data->height;

            if(region.bin_packer->AddRectangle(bin_rect))
            {
                new_entry_id = m_entry_id_gen++;

                // Update the atlas texture
                auto& texture = m_texture_set->list_texture_desc[0].first;

                texture->UpdateTexture(
                            ks::gl::Texture2D::Update{
                                ks::gl::Texture2D::Update::Defaults,
                                glm::u16vec2(
                                    region.x + bin_rect.x,
                                    region.y + bin_rect.y),
                                image_data
                            });

                // Save entry
                if(source.lifetime == Source::Lifetime::Permanent)
                {
                    region.lkup_entries.emplace(
                                new_entry_id,
                                Entry{
                                    std::move(source),
                                    bin_rect,
                                    nullptr
                                });
                }
                else
                {
                    source.load = nullptr;

                    region.lkup_entries.emplace(
                                new_entry_id,
                                Entry{
                                    std::move(source),
                                    bin_rect,
                                    image_data
                                });
                }

                return new_entry_id;
            }
        }

        throw ImageAtlasNoSpaceAvail();
    }

    void ImageAtlas::RemoveImage(Id image_id)
    {
        for(auto& region : m_list_regions)
        {
            if(region.lkup_entries.erase(image_id) > 0)
            {
                if(region.lkup_entries.empty())
                {
                    // Clear the region so we can add images to it
                    uint region_w = region.bin_packer->GetWidth();
                    uint region_h = region.bin_packer->GetHeight();

                    region.bin_packer =
                            make_unique<ks::BinPackShelf>(
                                region_w,
                                region_h,
                                1);
                }
                break;
            }
        }
    }

    ImageAtlas::ImageDesc ImageAtlas::GetImage(Id image_id) const
    {
        for(auto& region : m_list_regions)
        {
            auto it = region.lkup_entries.find(image_id);
            if(it != region.lkup_entries.end())
            {
                auto& rect = it->second.rect;

                uint x0 = rect.x+region.x;
                uint x1 = x0+rect.width;
                uint y0 = rect.y+region.y;
                uint y1 = y0+rect.height;

                ImageDesc image_desc{
                    m_texture_set_id,
                    x0/float(m_width_px),
                    y0/float(m_height_px),
                    x1/float(m_width_px),
                    y1/float(m_height_px)
                };

                return image_desc;
            }
        }

        throw ImageAtlasIdNotFound();
    }

    // ============================================================= //
    // ============================================================= //
}
