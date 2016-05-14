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

#ifndef RAINTK_ATLAS_IMAGE_HPP
#define RAINTK_ATLAS_IMAGE_HPP

#include <raintk/RainTkDrawableWidget.hpp>

namespace raintk
{
    class ImageAtlas;

    // AtlasImage
    // * Widget that draws an image using an ImageAtlas
    class AtlasImage : public DrawableWidget
    {
    public:
        AtlasImage(ks::Object::Key const &key,
                   Scene* scene,
                   shared_ptr<Widget> parent,
                   shared_ptr<ImageAtlas> image_atlas,
                   Id atlas_image_id);

        void Init(ks::Object::Key const &,
                  shared_ptr<AtlasImage> const &);

        ~AtlasImage();

    private:
        void onWidthChanged() override;
        void onHeightChanged() override;
        void onClipIdUpdated() override;
        void onTransformUpdated() override;
        void onOpacityChanged() override;
        void createDrawables() override;
        void destroyDrawables() override;
        void updateDrawables() override;
        void updateGeometry();
        static void setupTypeInit(Scene* scene);

        DrawDataComponentList* const m_cmlist_draw_data;
        shared_ptr<ImageAtlas> const m_image_atlas;
        Id const m_atlas_image_id;
        bool m_upd_geometry;
    };
}


#endif // RAINTK_ATLAS_IMAGE_HPP
