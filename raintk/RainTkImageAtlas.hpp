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

#ifndef RAINTK_IMAGE_ATLAS_HPP
#define RAINTK_IMAGE_ATLAS_HPP

#include <map>
#include <ks/KsException.hpp>
#include <ks/shared/KsBinPackShelf.hpp>
#include <ks/gl/KsGLTexture2D.hpp>
#include <raintk/RainTkGlobal.hpp>

namespace ks
{
    namespace draw
    {
        class TextureSet;
    }
}

namespace raintk
{
    class Scene;

    // ============================================================= //
    // ============================================================= //

    class ImageAtlasNoSpaceAvail : public ks::Exception
    {
    public:
        ImageAtlasNoSpaceAvail();
        ~ImageAtlasNoSpaceAvail() = default;
    };

    class ImageAtlasIdNotFound : public ks::Exception
    {
    public:
        ImageAtlasIdNotFound();
        ~ImageAtlasIdNotFound() = default;
    };

    // ============================================================= //
    // ============================================================= //

    // ImageAtlas
    // * Represents an atlas that combines image data into
    //   a single texture
    // * Can be used to avoid lots of texture state switches to
    //   increase performance when rendering lots of small images
    // * The AtlasImage class shows a basic way to use the atlas
    class ImageAtlas
    {
    public:
        struct Source
        {
            enum class Lifetime
            {
                Permanent,
                Temporary // keeps a copy of the data
            };

            Lifetime lifetime;
            std::function<shared_ptr<ks::ImageData>()> load;

            // If lifetime == Temporary
            // * keep a copy of the data
            // * set load fn = nullptr
        };

        struct ImageDesc
        {
            Id texture_set_id;
            float s0;
            float t0;
            float s1;
            float t1;
        };

    private:
        struct Entry
        {
            Source source;
            ks::BinPackRectangle rect;
            shared_ptr<ks::ImageData> backup;
        };

        struct Region
        {
            // top left corner
            uint x;
            uint y;

            unique_ptr<ks::BinPackShelf> bin_packer;
            std::map<Id,Entry> lkup_entries;
        };

    public:
        ImageAtlas(Scene* scene,
                   uint width_px=512,
                   uint height_px=512,
                   uint x_regions=2,
                   uint y_regions=2,
                   ks::gl::Texture2D::Format format=
                        ks::gl::Texture2D::Format::RGBA8);

        ~ImageAtlas();

        // * Adds an image to the atlas
        // * Returns a unique id to remove or lookup the image
        // * Will throw NoSpaceAvail if no space is available
        Id AddImage(Source source);

        // * Removes the specified image from the atlas
        void RemoveImage(Id image_id);

        // * Returns the description required to render the
        //   specified image
        // * Throws IdNotFound if the image can't be found
        ImageDesc GetImage(Id image_id) const;

    private:
        Scene* const m_scene;
        uint const m_width_px;
        uint const m_height_px;
        uint const m_x_regions;
        uint const m_y_regions;
        ks::gl::Texture2D::Format const m_format;

        uint m_entry_id_gen{1};

        Id m_texture_set_id;
        shared_ptr<ks::draw::TextureSet> m_texture_set;

        std::vector<Region> m_list_regions;
    };

    // ============================================================= //
    // ============================================================= //
}

#endif // RAINTK_IMAGE_ATLAS_HPP
