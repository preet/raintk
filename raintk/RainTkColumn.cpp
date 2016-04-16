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

#include <raintk/RainTkColumn.hpp>
#include <raintk/RainTkScene.hpp>

namespace raintk
{
    Column::Column(ks::Object::Key const &key,
                   Scene* scene,
                   shared_ptr<Widget> parent) :
        Widget(key,scene,parent)
    {

    }

    void Column::Init(ks::Object::Key const &,
                      shared_ptr<Column> const &this_col)
    {
        m_cid_spacing =
                spacing.signal_changed.Connect(
                    this_col,
                    &Column::onSpacingChanged,
                    ks::ConnectionType::Direct);
    }

    Column::~Column()
    {

    }

    void Column::AddChild(shared_ptr<Widget> const &child)
    {
        Widget::AddChild(child);

        shared_ptr<Column> this_col =
                std::static_pointer_cast<Column>(
                    shared_from_this());

        Item new_item;
        new_item.widget = child.get();

        new_item.cid_height =
                child->height.signal_changed.Connect(
                    this_col,
                    &Column::onChildDimsChanged,
                    ks::ConnectionType::Direct);

        // Save
        auto it = m_list_items.insert(m_list_items.end(),new_item);
        m_lkup_id_item_it.emplace(child->GetId(),it);

        // Mark this widget as updated
        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateWidget;
    }

    void Column::RemoveChild(shared_ptr<Widget> const &child)
    {
        auto lkup_it = m_lkup_id_item_it.find(child->GetId());
        if(lkup_it != m_lkup_id_item_it.end())
        {
            auto it = lkup_it->second;

            it->widget->width.signal_changed.Disconnect(
                        it->cid_height);

            m_list_items.erase(it);
            m_lkup_id_item_it.erase(lkup_it);

            m_cmlist_update_data->GetComponent(m_entity_id).
                    update |= UpdateData::UpdateWidget;
        }

        Widget::RemoveChild(child);
    }

    void Column::onSpacingChanged()
    {
        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateWidget;
    }

    void Column::onChildDimsChanged()
    {
        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateWidget;
    }

    void Column::update()
    {
        uint count = 0;
        float bottom = 0;
        float spacing_val = spacing.Get();

        for(auto& item : m_list_items)
        {
            item.widget->y = count*spacing_val + bottom;

            count++;
            bottom += item.widget->height.Get();
        }
    }
}
