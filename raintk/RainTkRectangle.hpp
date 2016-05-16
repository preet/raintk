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

#ifndef RAINTK_RECTANGLE_HPP
#define RAINTK_RECTANGLE_HPP

#include <raintk/RainTkDrawableWidget.hpp>

namespace raintk
{
    class Rectangle : public DrawableWidget
    {
    public:
        using base_type = DrawableWidget;

        Rectangle(ks::Object::Key const &key,
                  Scene* scene,
                  shared_ptr<Widget> parent);

        void Init(ks::Object::Key const &,
                  shared_ptr<Rectangle> const &);

        ~Rectangle();


        // Properties
        Property<glm::u8vec4> color {
            glm::u8vec4{255,51,51,255}
        };

    protected:
        virtual void onColorChanged();

        void onWidthChanged() override;
        void onHeightChanged() override;

        void onClipIdUpdated() override;
        void onTransformUpdated() override;
        void onAccOpacityUpdated() override;

        void createDrawables() override;
        void destroyDrawables() override;
        void updateDrawables() override;

        DrawDataComponentList* const m_cmlist_draw_data;

        Id m_cid_color;

    private:
        void updateGeometry();
        static void setupTypeInit(Scene* scene);
    };
}

#endif // RAINTK_RECTANGLE_HPP
