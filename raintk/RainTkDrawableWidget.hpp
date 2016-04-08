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

#ifndef RAINTK_DRAWABLE_WIDGET_HPP
#define RAINTK_DRAWABLE_WIDGET_HPP

#include <atomic>
#include <raintk/RainTkWidget.hpp>
#include <raintk/RainTkLog.hpp>

namespace raintk
{
    class DrawSystem;

    class DrawableWidget : public Widget
    {
    public:
        using base_type = Widget;

        friend class raintk::Scene;
        friend class raintk::DrawSystem;

        DrawableWidget(ks::Object::Key const &key,
                      shared_ptr<Widget> parent,
                      std::string name);

        void Init(ks::Object::Key const &,
                  shared_ptr<DrawableWidget> const &);

        ~DrawableWidget();

        bool GetIsDrawable() const final;

        // Properties
        Property<bool> visible {
            name+".visible",true
        };

        Property<float> opacity {
            name+".opacity",1.0f
        };

        template<typename T>
        static Id GetTypeId()
        {
            static uint id = s_type_id_count++;
            return id;
        }

        template<typename T>
        static void SetInitCallback(std::function<void()> init_fn)
        {
            rtklog.Trace() << "SetInitCallback <" <<
                        DrawableWidget::GetTypeId<T>() << ">";

            s_lkup_renderable_init.emplace(
                        DrawableWidget::GetTypeId<T>(),
                        std::move(init_fn));
        }

    protected:
        virtual void onVisibilityChanged();
        virtual void onOpacityChanged();

        // * Create the renderable components for this
        //   Widget (like RenderData or BatchData)
        virtual void createDrawables() = 0;

        // TODO desc
        virtual void destroyDrawables() = 0;

        //
        virtual void updateDrawables() = 0;


        Id m_cid_visible;
        Id m_cid_opacity;

    private:
        static uint s_type_id_count;
        static std::map<Id,std::function<void()>> s_lkup_renderable_init;
    };

}

#endif // RAINTK_DRAWABLE_WIDGET_HPP
