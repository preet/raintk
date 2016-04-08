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

#ifndef RAINTK_LIST_MODEL_STL_VECTOR_HPP
#define RAINTK_LIST_MODEL_STL_VECTOR_HPP

#include <raintk/RainTkListModel.hpp>

namespace raintk
{
    // rn ListModelVector?
    template<typename DataType>
    class ListModelSTLVector final : public ListModel<DataType>
    {
    public:
        ListModelSTLVector()
        {}

        ~ListModelSTLVector()
        {}


        uint GetSize() const override
        {
            return m_list.size();
        }

        DataType const & GetData(uint index) const override
        {
            return m_list[index];
        }


        // This model can write to its items
        void SetData(uint index, DataType const &data) override
        {
            m_list[index] = data;
            this->signal_data_changed.Emit(index);
        }


        // This model can be resized
        void Insert(uint idx_before, DataType const &data) override
        {
            this->signal_before_adding_items.Emit(idx_before,1);
            m_list.insert(std::next(m_list.begin(),idx_before),data);
            this->signal_added_items.Emit(idx_before,idx_before+1);
        }

        void Insert(uint idx_before, uint count, DataType const &data) override
        {
            this->signal_before_adding_items.Emit(idx_before,count);
            m_list.insert(std::next(m_list.begin(),idx_before),count,data);
            this->signal_added_items.Emit(idx_before,idx_before+count);
        }

        void Insert(uint idx_before, std::vector<DataType> const &list_data) override
        {
            this->signal_before_adding_items.Emit(idx_before,list_data.size());
            m_list.insert(std::next(m_list.begin(),idx_before),
                          list_data.begin(),
                          list_data.end());

            this->signal_added_items.Emit(
                        idx_before,
                        idx_before+list_data.size());
        }

        void Erase(uint idx) override
        {
            this->signal_before_removing_items.Emit(idx,idx+1);
            m_list.erase(std::next(m_list.begin(),idx));
            this->signal_removed_items.Emit(idx,1);
        }

        void Erase(uint idx_first, uint idx_after_last) override
        {
            this->signal_before_removing_items.Emit(idx_first,idx_after_last);
            m_list.erase(std::next(m_list.begin(),idx_first),
                         std::next(m_list.begin(),idx_after_last));

            this->signal_removed_items.Emit(
                        idx_first,
                        idx_after_last-idx_first);
        }


        // Supported vector methods
        void Reserve(uint capacity)
        {
            m_list.reserve(capacity);
        }

        uint Capacity() const
        {
            return m_list.capacity();
        }

        void ShrinkToFit()
        {
            m_list.shrink_to_fit();
        }

        void Clear()
        {
            Erase(0,m_list.size());
        }

        void PushBack(DataType const &data)
        {
            Insert(m_list.size(),data);
        }

        void Resize(uint new_size)
        {
            if(new_size == 0)
            {
                Clear();
                return;
            }

            if(new_size < m_list.size())
            {
                Erase(new_size,m_list.size());
            }
        }

    private:
        std::vector<DataType> m_list;
    };
}

#endif // RAINTK_LIST_MODEL_STL_VECTOR_HPP
