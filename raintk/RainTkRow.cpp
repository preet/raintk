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

#include <raintk/RainTkRow.hpp>
#include <raintk/RainTkScene.hpp>

namespace raintk
{
    Row::Row(ks::Object::Key const &key,
             Scene* scene,
             shared_ptr<Widget> parent) :
        Widget(key,scene,parent)
    {

    }

    void Row::Init(ks::Object::Key const &,
                   shared_ptr<Row> const &this_row)
    {
        m_cid_spacing =
                spacing.signal_changed.Connect(
                    this_row,
                    &Row::onSpacingChanged,
                    ks::ConnectionType::Direct);

        m_cid_layout_direction =
                layout_direction.signal_changed.Connect(
                    this_row,
                    &Row::onLayoutDirectionChanged,
                    ks::ConnectionType::Direct);
    }

    Row::~Row()
    {

    }

    void Row::AddChild(shared_ptr<Widget> const &child)
    {
        Widget::AddChild(child);

        shared_ptr<Row> this_row =
                std::static_pointer_cast<Row>(
                    shared_from_this());

        Item new_item;
        new_item.widget = child.get();

        new_item.cid_width =
                child->width.signal_changed.Connect(
                    this_row,
                    &Row::onChildDimsChanged,
                    ks::ConnectionType::Direct);

        new_item.cid_height =
                child->height.signal_changed.Connect(
                    this_row,
                    &Row::onChildDimsChanged,
                    ks::ConnectionType::Direct);

        // Save in row list
        auto it = m_list_items.insert(
                    m_list_items.end(),
                    new_item);

        m_lkup_id_item_it.emplace(child->GetId(),it);

        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateWidget;
    }

    void Row::RemoveChild(shared_ptr<Widget> const &child)
    {
        auto lkup_it = m_lkup_id_item_it.find(child->GetId());
        if(lkup_it != m_lkup_id_item_it.end())
        {
            auto it = lkup_it->second;

            it->widget->width.signal_changed.Disconnect(it->cid_width);
            it->widget->height.signal_changed.Disconnect(it->cid_height);

            m_list_items.erase(it);
            m_lkup_id_item_it.erase(lkup_it);

            m_cmlist_update_data->GetComponent(m_entity_id).
                    update |= UpdateData::UpdateWidget;
        }

        Widget::RemoveChild(child);
    }

    void Row::onSpacingChanged()
    {
        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateWidget;
    }

    void Row::onLayoutDirectionChanged()
    {
        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateWidget;
    }

    void Row::onChildDimsChanged()
    {
        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateWidget;
    }

    void Row::update()
    {
        float const spacing_val = spacing.Get();
        float row_height=0;
        float left = 0;

        for(auto& item : m_list_items)
        {
            item.widget->x = left;
            left += (item.widget->width.Get()+spacing_val);
            row_height = std::max(item.widget->height.Get(),row_height);
        }

        float const row_width = left-spacing_val;
        children_width = row_width;
        children_height = row_height;

        if(layout_direction.Get() == LayoutDirection::RightToLeft)
        {
            for(auto& item : m_list_items)
            {
                float right =
                        item.widget->x.Get() +
                        item.widget->width.Get();

                item.widget->x = (right*-1.0)+row_width;
            }
        }
    }
}
