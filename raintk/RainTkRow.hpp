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

#ifndef RAINTK_ROW_HPP
#define RAINTK_ROW_HPP

#include <map>
#include <list>
#include <raintk/RainTkWidget.hpp>

namespace raintk
{
    class Row : public Widget
    {
    public:
        using base_type = raintk::Widget;

        enum class LayoutDirection
        {
            LeftToRight,
            RightToLeft
        };

        Row(ks::Object::Key const &key,
            Scene* scene,
            shared_ptr<Widget> parent);

        void Init(ks::Object::Key const &,
                  shared_ptr<Row> const &);

        ~Row();

        void AddChild(shared_ptr<Widget> const &child) override;
        void RemoveChild(shared_ptr<Widget> const &child) override;

        // Properties
        Property<float> spacing{
            0.0f
        };

        Property<LayoutDirection> layout_direction{
            LayoutDirection::LeftToRight
        };

    protected:
        void onSpacingChanged();
        void onLayoutDirectionChanged();
        void onChildDimsChanged();

        Id m_cid_spacing;
        Id m_cid_layout_direction;

    private:
        void update() override;

        struct Item
        {
            Widget* widget;
            Id cid_width;
        };

        // TODO replace list if performance is an issue
        std::list<Item> m_list_items;
        std::map<Id,std::list<Item>::iterator> m_lkup_id_item_it;
    };
}

#endif // RAINTK_ROW_HPP
