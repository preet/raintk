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

#ifndef RAINTK_LIST_DELEGATE_HPP
#define RAINTK_LIST_DELEGATE_HPP

#include <raintk/RainTkWidget.hpp>

namespace raintk
{
    // NOTE:
    // Classes that derive from ListDelegate should
    // have a method with the signature:

    // void SetData(ItemType const &);

    // We don't currently check for this (it just won't
    // compile if the method is left out) but if we want
    // to in the future, then CRTP can be used

    // template<typename ItemType, typename DerivedType>
    // class ListDelegate
    // {
    //    static_assert:
    //      * DerivedType inherits from ListDelegate
    //      * DerivedType contains a method with the
    //        signature "void SetData(ItemType const &); "
    // };

    class ListDelegate : public Widget
    {
        template<typename ItemType, typename DelegateType>
        friend class ListView;

    public:
        using base_type = Widget;

        ListDelegate(ks::Object::Key const &key,
                     shared_ptr<Widget> parent) :
            Widget(key,parent,"")
        {}

        void Init(ks::Object::Key const &,
                  shared_ptr<ListDelegate> const &)
        {}

        virtual ~ListDelegate()
        {
            width.signal_changed.Disconnect(m_cid_delegate_width);
            height.signal_changed.Disconnect(m_cid_delegate_height);
        }

        virtual void SetIndex(uint index) = 0;
        virtual uint GetIndex() const = 0;

        // TODO add animations
        // virtual void StartAddAnimation() = 0;
        // virtual void StartRemoveAnimation() = 0;

        //virtual void SetSelected(bool selected) = 0;
        //virtual void GetSelected() const = 0;

    private:
        Id m_cid_delegate_width;
        Id m_cid_delegate_height;
    };
}

#endif // RAINTK_LIST_DELEGATE_HPP

