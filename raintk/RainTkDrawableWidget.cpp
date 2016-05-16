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

#include <raintk/RainTkDrawableWidget.hpp>

namespace raintk
{
    uint DrawableWidget::s_type_id_count = 1;

    std::map<Id,std::function<void()>> DrawableWidget::s_lkup_renderable_init;

    DrawableWidget::DrawableWidget(ks::Object::Key const &key,
                                   Scene* scene,
                                   shared_ptr<Widget> parent) :
        Widget(key,scene,parent)
    {

    }

    void DrawableWidget::Init(ks::Object::Key const &,
                             shared_ptr<DrawableWidget> const &this_widget)
    {
        // Connect properties
        m_cid_visible =
                visible.signal_changed.Connect(
                    this_widget,
                    &DrawableWidget::onVisibilityChanged,
                    ks::ConnectionType::Direct);
    }

    DrawableWidget::~DrawableWidget()
    {

    }

    bool DrawableWidget::GetIsDrawable() const
    {
        return true;
    }

    void DrawableWidget::onVisibilityChanged()
    {

    }
}
