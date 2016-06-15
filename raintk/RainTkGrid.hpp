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

#ifndef RAINTK_GRID_HPP
#define RAINTK_GRID_HPP

#include <map>
#include <list>
#include <raintk/RainTkWidget.hpp>

namespace raintk
{
    // =========================================================== //

    class GridDimensionInvalid : public ks::Exception
    {
    public:
        GridDimensionInvalid(std::string msg);
        ~GridDimensionInvalid() = default;
    };

    // =========================================================== //

    class Grid : public Widget
    {
    public:
        using base_type = raintk::Widget;

        enum class LayoutDirection
        {
            LeftToRight,
            RightToLeft,
            TopToBottomLTR,
            TopToBottomRTL
        };

        Grid(ks::Object::Key const &key,
             Scene* scene,
             shared_ptr<Widget> parent);

        void Init(ks::Object::Key const &,
                  shared_ptr<Grid> const &);

        ~Grid();

        void AddChild(shared_ptr<Widget> const &child) override;
        void RemoveChild(shared_ptr<Widget> const &child) override;

        // Properties
        Property<LayoutDirection> layout_direction{
            LayoutDirection::LeftToRight
        };

        Property<uint> rows{
            3
        };

        Property<uint> cols{
            3
        };

        Property<float> row_spacing{
            0.0f
        };

        Property<float> col_spacing{
            0.0f
        };

        Property<float> children_width{
            0.0f
        };

        Property<float> children_height{
            0.0f
        };

    protected:
        void onLayoutChanged();

        Id m_cid_layout_direction;
        Id m_cid_rows;
        Id m_cid_cols;
        Id m_cid_row_spacing;
        Id m_cid_col_spacing;

    private:
        void update() override;

        struct Item
        {
            Widget* widget;
            Id cid_height;
            Id cid_width;
        };

        std::list<Item> m_list_items;
        std::map<Id,std::list<Item>::iterator> m_lkup_id_item_it;
    };

    // =========================================================== //
}

#endif // RAINTK_GRID_HPP
