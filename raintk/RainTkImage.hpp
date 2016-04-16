/*
   Copyright (C) 2015 Preet Desai (preet.desai@gmail.com)

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

#ifndef RAINTK_IMAGE_HPP
#define RAINTK_IMAGE_HPP

#include <raintk/RainTkDrawableWidget.hpp>

namespace ks
{
    struct ImageData;

    namespace draw
    {
        struct UniformSet;
        struct TextureSet;
    }
}

namespace raintk
{
    class Image : public DrawableWidget
    {
    public:
        using base_type = DrawableWidget;

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

        enum class FillMode
        {
            Stretch,
            Tile
        };

        Image(ks::Object::Key const &key,
              Scene* scene,
              shared_ptr<Widget> parent);

        void Init(ks::Object::Key const &,
                  shared_ptr<Image> const &);

        ~Image();


        // Properties
        Property<Source> source {
            Source{}
        };

        Property<FillMode> fill_mode {
            FillMode::Stretch
        };

        Property<bool> smooth {
            true
        };


    protected:
        void onWidthChanged() override;
        void onHeightChanged() override;
        void onOpacityChanged() override;
        void onClipIdUpdated() override;
        void onTransformUpdated() override;
        void onSourceChanged();
        void onFillModeChanged();
        void onSmoothChanged();

        void createDrawables() override;
        void destroyDrawables() override;
        void updateDrawables() override;

        DrawDataComponentList* const m_cmlist_draw_data;

        Id m_cid_source;
        Id m_cid_fill_mode;

    private:
        void updateGeometry();
        void updateTransform();
        void updateTexture();
        void updateTileRatio();
        void updateSmooth();
        static void setupTypeInit(Scene* scene);

        Id m_uniform_set_id;
        Id m_texture_set_id;
        shared_ptr<ks::draw::UniformSet> m_uniform_set;
        shared_ptr<ks::draw::TextureSet> m_texture_set;

        glm::vec2 m_image_data_size_px;
        shared_ptr<ks::ImageData> m_image_data;

        bool m_upd_recreate;
        bool m_upd_geometry;
        bool m_upd_xf;
        bool m_upd_texture;
        bool m_upd_tile_ratio;
        bool m_upd_smooth;
    };
}

#endif // RAINTK_IMAGE_HPP

