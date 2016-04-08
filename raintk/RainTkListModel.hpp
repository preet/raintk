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

#ifndef RAINTK_LIST_MODEL_HPP
#define RAINTK_LIST_MODEL_HPP

#include <vector>
#include <ks/KsSignal.hpp>
#include <raintk/RainTkGlobal.hpp>

namespace raintk
{
    class ListModelItemsAreReadOnly : public ks::Exception
    {
    public:
        ListModelItemsAreReadOnly();
        ~ListModelItemsAreReadOnly() = default;
    };

    class ListModelSizeIsFixed : public ks::Exception
    {
    public:
        ListModelSizeIsFixed();
        ~ListModelSizeIsFixed() = default;
    };


    template<typename T>
    class ListModel
    {
    public:
        // uint: index before which items will be added
        // uint: number of items added
        ks::Signal<uint,uint> signal_before_adding_items;

        // uint: index to first item added
        // uint: index to one past the last item added
        ks::Signal<uint,uint> signal_added_items;

        // uint: index to the first item to be removed
        // uint: index to one past the last item to be removed
        ks::Signal<uint,uint> signal_before_removing_items;

        // uint: index after the last removed item
        // uint: number of items removed
        ks::Signal<uint,uint> signal_removed_items;

        // uint: index
        ks::Signal<uint> signal_data_changed;

        // emitted when the model data changes substantially
        // (like after a sort)
        ks::Signal<> signal_layout_changed;


        ListModel() {}

        ~ListModel() {}

        // Every model must implement the following two methods
        virtual uint GetSize() const = 0;
        virtual T const & GetData(uint index) const = 0;

        // Reimplement these for a model that can modify its items
        virtual void SetData(uint index, T const &data)
        {
            (void)index;
            (void)data;
            throw ListModelItemsAreReadOnly();
        }

        // Reimplement these for a model that can be resized
        virtual void Insert(uint idx_before, T const &data)
        {
            (void)idx_before;
            (void)data;
            throw ListModelSizeIsFixed();
        }

        virtual void Insert(uint idx_before, uint count, T const &data)
        {
            (void)idx_before;
            (void)count;
            (void)data;
            throw ListModelSizeIsFixed();
        }

        virtual void Insert(uint idx_before, std::vector<T> const &list_data)
        {
            (void)idx_before;
            (void)list_data;
            throw ListModelSizeIsFixed();
        }

        virtual void Erase(uint idx)
        {
            (void)idx;
            throw ListModelSizeIsFixed();
        }

        virtual void Erase(uint idx_first, uint idx_after_last)
        {
            (void)idx_first;
            (void)idx_after_last;
            throw ListModelSizeIsFixed();
        }

    };


//    template<typename T>
//    class ListModelOld
//    {
//    public:
//        // Iterator to underlying container. All iterators
//        // are unstable and can be invalidated when the model
//        // is modified
//        using Iterator = typename std::vector<T>::iterator;

//        ks::Signal<uint> signal_size_changed;

//        // Iterator: iterator before which items will be added
//        // uint: number of items added
//        ks::Signal<Iterator,uint> signal_before_adding_items;

//        // Iterator: iterator to first item added
//        // Iterator: iterator to one past the last item added
//        ks::Signal<Iterator,Iterator> signal_added_items;

//        // Iterator: iterator to the first item to be removed
//        // Iterator: iterator to one past the last item to be removed
//        ks::Signal<Iterator,Iterator> signal_before_removing_items;

//        // Iterator: iterator after the last removed item
//        // uint: number of items removed
//        ks::Signal<Iterator,uint> signal_removed_items;


//        ListModel()
//        {

//        }

//        ~ListModel()
//        {
//            Clear();
//        }

//        // Iterators
//        Iterator Begin()
//        {
//            return m_list.begin();
//        }

//        Iterator End()
//        {
//            return m_list.end();
//        }

//        // Capacity
//        uint Size() const
//        {
//            return m_list.size();
//        }

//        void Reserve(uint capacity)
//        {
//            m_list.reserve(capacity);
//        }

//        uint Capacity() const
//        {
//            return m_list.capacity();
//        }

//        void ShrinkToFit()
//        {
//            m_list.shrink_to_fit();
//        }

//        // Modifiers
//        void Clear()
//        {
//            Erase(m_list.begin(),m_list.end());
//        }

//        Iterator Insert(Iterator pos, T const &value)
//        {
//            signal_before_adding_items.Emit(pos,1);
//            auto it = m_list.insert(pos,value);
//            signal_added_items.Emit(it,std::next(it));
//            return it;
//        }

//        Iterator Insert(Iterator pos, T&& value)
//        {
//            signal_before_adding_items.Emit(pos,1);
//            auto it = m_list.insert(pos,value);
//            signal_added_items.Emit(it,std::next(it));
//            return it;
//        }

//        Iterator Insert(Iterator pos, uint count, T const &value)
//        {
//            signal_before_adding_items.Emit(pos,count);
//            auto it = m_list.insert(pos,count,value);
//            signal_added_items.Emit(it,std::next(it,count));
//            return it;
//        }

//        template<class InputIt>
//        Iterator Insert(Iterator pos, InputIt first, InputIt last)
//        {
//            auto const count = last-first;
//            signal_before_adding_items.Emit(pos,count);
//            auto it = m_list.insert(pos,first,last);
//            signal_added_items.Emit(it,std::next(it,count));
//            return it;
//        }

//        Iterator Erase(Iterator pos)
//        {
//            signal_before_removing_items.Emit(pos,std::next(pos,1));
//            auto it = m_list.erase(pos);
//            signal_removed_items.Emit(it,1);
//            return it;
//        }

//        Iterator Erase(Iterator first, Iterator last)
//        {
//            auto const count = last-first;
//            signal_before_removing_items.Emit(first,last);
//            auto it = m_list.erase(first,last);
//            signal_removed_items.Emit(it,count);
//            return it;
//        }

//        void Resize(uint new_size)
//        {
//            // TODO use Erase instead of .resize
//            m_list.resize(new_size);
//            signal_size_changed.Emit(0);
//        }

//    private:
//        std::vector<T> m_list;
//        // std::vector<std::pair<Iterator,Iterator>> m_list_selection_ranges;
//    };
}

#endif // RAINTK_LIST_MODEL_HPP
