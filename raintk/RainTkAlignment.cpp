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

#include <raintk/RainTkWidget.hpp>
#include <raintk/RainTkAlignment.hpp>
#include <raintk/RainTkLog.hpp>

namespace raintk
{
    WidgetAlignmentInvalid::WidgetAlignmentInvalid(std::string msg) :
        ks::Exception(ks::Exception::ErrorLevel::ERROR,std::move(msg),false)
    {}

    namespace Align
    {
        // =========================================================== //

        namespace
        {
            void EnforceParentOrSibling(
                    shared_ptr<Widget> const &widget,
                    shared_ptr<Widget> const &anchor)
            {
                bool sibling =
                        (widget != anchor) &&
                        (widget->GetParent() == anchor->GetParent());

                bool parent =
                        (widget->GetParent() == anchor);

                if(!(sibling || parent))
                {
                    throw WidgetAlignmentInvalid(
                                "WidgetAlignment: "
                                "Anchor must be sibling or parent");
                }
            }
        }

        // =========================================================== //

        void AssignCenterToAnchorCenter(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->x = 0.5f*(anchor->width.Get() - widget->width.Get());
                widget->y = 0.5f*(anchor->height.Get() - widget->height.Get());
            }
            else
            {
                widget->x = anchor->x.Get() + 0.5f*(anchor->width.Get() - widget->width.Get());
                widget->y = anchor->y.Get() + 0.5f*(anchor->height.Get() - widget->height.Get());
            }
        }

        // Horizontal
        void AssignHCenterToAnchorHCenter(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->x = 0.5f*(anchor->width.Get() - widget->width.Get());
            }
            else
            {
                widget->x = anchor->x.Get() + 0.5f*(anchor->width.Get() - widget->width.Get());
            }
        }

        void AssignRightToAnchorLeft(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->x = -1.0f*widget->width.Get();
            }
            else
            {
                widget->x = anchor->x.Get() - widget->width.Get();
            }
        }

        void AssignLeftToAnchorLeft(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->x = 0.0f;
            }
            else
            {
                widget->x = anchor->x.Get();
            }
        }

        void AssignLeftToAnchorRight(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->x = anchor->width.Get();
            }
            else
            {
                widget->x = anchor->x.Get() + anchor->width.Get();
            }
        }

        void AssignRightToAnchorRight(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->x = anchor->width.Get() - widget->width.Get();
            }
            else
            {
                widget->x = anchor->x.Get() + anchor->width.Get() - widget->width.Get();
            }
        }

        // Vertical
        void AssignVCenterToAnchorVCenter(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->y = 0.5f*(anchor->height.Get() - widget->height.Get());
            }
            else
            {
                widget->y = anchor->y.Get() + 0.5f*(anchor->height.Get() - widget->height.Get());
            }
        }

        void AssignBottomToAnchorTop(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->y = 0.0f - widget->height.Get();
            }
            else
            {
                widget->y = anchor->y.Get() - widget->height.Get();
            }
        }

        void AssignTopToAnchorTop(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->y = 0.0f;
            }
            else
            {
                widget->y = anchor->y.Get();
            }
        }

        void AssignBottomToAnchorBottom(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->y = anchor->height.Get() - widget->height.Get();
            }
            else
            {
                widget->y = anchor->y.Get() + anchor->height.Get() - widget->height.Get();
            }
        }

        void AssignTopToAnchorBottom(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->y = anchor->height.Get();
            }
            else
            {
                widget->y = anchor->y.Get() + anchor->height.Get();
            }
        }

        // =========================================================== //

        void BindCenterToAnchorCenter(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->x = [&](){ return (0.5f*(anchor->width.Get() - widget->width.Get())); };
                widget->y = [&](){ return (0.5f*(anchor->height.Get() - widget->height.Get())); };
            }
            else
            {
                widget->x = [&](){ return (anchor->x.Get() + 0.5f*(anchor->width.Get() - widget->width.Get())); };
                widget->y = [&](){ return (anchor->y.Get() + 0.5f*(anchor->height.Get() - widget->height.Get())); };
            }
        }

        // Horizontal
        void BindHCenterToAnchorHCenter(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                [&](){ return (widget->x = 0.5f*(anchor->width.Get() - widget->width.Get())); };
            }
            else
            {
                [&](){ return (widget->x = anchor->x.Get() + 0.5f*(anchor->width.Get() - widget->width.Get())); };
            }
        }

        void BindRightToAnchorLeft(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->x = [&](){ return -1.0f*widget->width.Get(); };
            }
            else
            {
                widget->x = [&](){ return (anchor->x.Get() - widget->width.Get()); };
            }
        }

        void BindLeftToAnchorLeft(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->x = [&](){ return 0.0f; };
            }
            else
            {
                widget->x = [&](){ return anchor->x.Get(); };
            }
        }

        void BindLeftToAnchorRight(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->x = [&](){ return anchor->width.Get(); };
            }
            else
            {
                widget->x = [&](){ return (anchor->x.Get() + anchor->width.Get()); };
            }
        }

        void BindRightToAnchorRight(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->x = [&](){ return (anchor->width.Get() - widget->width.Get()); };
            }
            else
            {
                widget->x = [&](){ return (anchor->x.Get() + anchor->width.Get() - widget->width.Get()); };
            }
        }

        // Vertical
        void BindVCenterToAnchorVCenter(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->y = [&](){ return (0.5f*(anchor->height.Get() - widget->height.Get())); };
            }
            else
            {
                widget->y = [&](){ return (anchor->y.Get() + 0.5f*(anchor->height.Get() - widget->height.Get())); };
            }
        }

        void BindBottomToAnchorTop(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->y = [&](){ return (0.0f - widget->height.Get()); };
            }
            else
            {
                widget->y = [&](){ return (anchor->y.Get() - widget->height.Get()); };
            }
        }

        void BindTopToAnchorTop(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->y = 0.0f;
            }
            else
            {
                widget->y = anchor->y.Get();
            }
        }

        void BindBottomToAnchorBottom(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->y = [&](){ return (anchor->height.Get() - widget->height.Get()); };
            }
            else
            {
                widget->y = [&](){ return (anchor->y.Get() + anchor->height.Get() - widget->height.Get()); };
            }
        }

        void BindTopToAnchorBottom(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent())
            {
                widget->y = anchor->height.Get();
            }
            else
            {
                widget->y = [&](){ return (anchor->y.Get() + anchor->height.Get()); };
            }
        }
    }
}
