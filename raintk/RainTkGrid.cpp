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

#include <raintk/RainTkGrid.hpp>
#include <raintk/RainTkScene.hpp>

#include <raintk/RainTkLog.hpp>

namespace raintk
{
    GridDimensionInvalid::GridDimensionInvalid(std::string msg) :
        ks::Exception(ks::Exception::ErrorLevel::ERROR,std::move(msg),false)
    {}

    // =========================================================== //

    Grid::Grid(ks::Object::Key const &key,
               Scene* scene,
               shared_ptr<Widget> parent) :
        Widget(key,scene,parent)
    {

    }

    void Grid::Init(ks::Object::Key const &,
                      shared_ptr<Grid> const &this_grid)
    {
        m_cid_layout_direction =
                layout_direction.signal_changed.Connect(
                    this_grid,
                    &Grid::onLayoutChanged,
                    ks::ConnectionType::Direct);

        m_cid_rows =
                rows.signal_changed.Connect(
                    this_grid,
                    &Grid::onLayoutChanged,
                    ks::ConnectionType::Direct);

        m_cid_cols =
                cols.signal_changed.Connect(
                    this_grid,
                    &Grid::onLayoutChanged,
                    ks::ConnectionType::Direct);

        m_cid_row_spacing =
                row_spacing.signal_changed.Connect(
                    this_grid,
                    &Grid::onLayoutChanged,
                    ks::ConnectionType::Direct);

        m_cid_col_spacing =
                col_spacing.signal_changed.Connect(
                    this_grid,
                    &Grid::onLayoutChanged,
                    ks::ConnectionType::Direct);
    }

    Grid::~Grid()
    {

    }

    void Grid::AddChild(shared_ptr<Widget> const &child)
    {
        Widget::AddChild(child);

        shared_ptr<Grid> this_grid =
                std::static_pointer_cast<Grid>(
                    shared_from_this());

        Item new_item;

        new_item.widget = child.get();

        new_item.cid_width =
                child->width.signal_changed.Connect(
                    this_grid,
                    &Grid::onLayoutChanged,
                    ks::ConnectionType::Direct);

        new_item.cid_height =
                child->height.signal_changed.Connect(
                    this_grid,
                    &Grid::onLayoutChanged,
                    ks::ConnectionType::Direct);

        // Save
        auto it = m_list_items.insert(m_list_items.end(),new_item);
        m_lkup_id_item_it.emplace(child->GetId(),it);

        // Mark this widget as updated
        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateWidget;
    }

    void Grid::RemoveChild(shared_ptr<Widget> const &child)
    {
        auto lkup_it = m_lkup_id_item_it.find(child->GetId());
        if(lkup_it != m_lkup_id_item_it.end())
        {
            auto it = lkup_it->second;

            it->widget->width.signal_changed.Disconnect(
                        it->cid_width);

            it->widget->height.signal_changed.Disconnect(
                        it->cid_height);

            m_list_items.erase(it);
            m_lkup_id_item_it.erase(lkup_it);

            m_cmlist_update_data->GetComponent(m_entity_id).
                    update |= UpdateData::UpdateWidget;
        }

        Widget::RemoveChild(child);
    }

    void Grid::onLayoutChanged()
    {
        m_cmlist_update_data->GetComponent(m_entity_id).
                update |= UpdateData::UpdateWidget;
    }

    void Grid::update()
    {
        auto const layout_dirn_val = layout_direction.Get();
        auto const row_spacing_val = row_spacing.Get();
        auto const col_spacing_val = col_spacing.Get();

        struct WidgetCell
        {
            uint c;
            uint r;
            Widget* widget;
        };

        std::vector<WidgetCell> list_widget_cells(m_list_items.size());
        std::vector<float> list_col_w;
        std::vector<float> list_row_h;

        // Create widget cells
        if(layout_dirn_val == LayoutDirection::LeftToRight ||
           layout_dirn_val == LayoutDirection::RightToLeft)
        {
            uint const col_count = cols.Get();
            if(col_count == 0) {
                throw GridDimensionInvalid(
                            "Grid: Column count must be greater than 0");
            }

            uint const full_row_count = (m_list_items.size()/col_count);

            list_col_w.resize(col_count,0);
            list_row_h.resize(full_row_count+1,0);

            uint item_count=0;

            for(auto& item : m_list_items)
            {
                auto& cell = list_widget_cells[item_count];
                cell.c = item_count%col_count;
                cell.r = item_count/col_count;
                cell.widget = item.widget;

                item_count++;
            }
        }
        else
        {
            uint const row_count = rows.Get();
            if(row_count == 0) {
                throw GridDimensionInvalid(
                            "Grid: Row count must be greater than 0");
            }

            uint const full_col_count = (m_list_items.size()/row_count);
            list_col_w.resize(full_col_count+1,0);
            list_row_h.resize(row_count,0);

            uint item_count=0;

            for(auto& item : m_list_items)
            {
                auto& cell = list_widget_cells[item_count];
                cell.c = item_count/row_count;
                cell.r = item_count%row_count;
                cell.widget = item.widget;

                item_count++;
            }
        }

        // Set cell dimensions based on max item dims
        for(auto& cell : list_widget_cells)
        {
            list_col_w[cell.c] =
                    std::max(
                        list_col_w[cell.c],
                        cell.widget->width.Get()+row_spacing_val);

            list_row_h[cell.r] =
                    std::max(
                        list_row_h[cell.r],
                        cell.widget->height.Get()+col_spacing_val);
        }

        // Accumulate cell dimensions
        float w_acc=0;
        for(auto& w : list_col_w)
        {
            w += w_acc;
            w_acc = w;
        }
        this->width = w_acc-row_spacing_val;

        float h_acc=0;
        for(auto& h : list_row_h)
        {
            h += h_acc;
            h_acc = h;
        }
        this->height = h_acc-col_spacing_val;

        // Position items
        if(layout_dirn_val == LayoutDirection::LeftToRight ||
           layout_dirn_val == LayoutDirection::TopToBottomLTR)
        {
            for(auto& cell : list_widget_cells)
            {
                cell.widget->x = (cell.c > 0) ? list_col_w[cell.c-1] : 0;
                cell.widget->y = (cell.r > 0) ? list_row_h[cell.r-1] : 0;
            }
        }
        else
        {
            float const grid_width = w_acc-row_spacing_val;

            for(auto& cell : list_widget_cells)
            {
                // Mirror x for RTL layouts
                float x = (cell.c > 0) ? list_col_w[cell.c-1] : 0;
                cell.widget->x = ((x+cell.widget->width.Get())*-1.0)+grid_width;
                cell.widget->y = (cell.r > 0) ? list_row_h[cell.r-1] : 0;
            }
        }
    }
}
