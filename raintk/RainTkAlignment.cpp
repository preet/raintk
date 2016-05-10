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
                    Widget* widget,
                    Widget* anchor)
            {
                bool sibling =
                        (widget != anchor) &&
                        (widget->GetParent() == anchor->GetParent());

                bool parent =
                        (widget->GetParent().get() == anchor);

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
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
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
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
            {
                widget->x = 0.5f*(anchor->width.Get() - widget->width.Get());
            }
            else
            {
                widget->x = anchor->x.Get() + 0.5f*(anchor->width.Get() - widget->width.Get());
            }
        }

        void AssignRightToAnchorLeft(
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
            {
                widget->x = -1.0f*widget->width.Get();
            }
            else
            {
                widget->x = anchor->x.Get() - widget->width.Get();
            }
        }

        void AssignLeftToAnchorLeft(
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
            {
                widget->x = 0.0f;
            }
            else
            {
                widget->x = anchor->x.Get();
            }
        }

        void AssignLeftToAnchorRight(
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
            {
                widget->x = anchor->width.Get();
            }
            else
            {
                widget->x = anchor->x.Get() + anchor->width.Get();
            }
        }

        void AssignRightToAnchorRight(
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
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
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
            {
                widget->y = 0.5f*(anchor->height.Get() - widget->height.Get());
            }
            else
            {
                widget->y = anchor->y.Get() + 0.5f*(anchor->height.Get() - widget->height.Get());
            }
        }

        void AssignBottomToAnchorTop(
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
            {
                widget->y = 0.0f - widget->height.Get();
            }
            else
            {
                widget->y = anchor->y.Get() - widget->height.Get();
            }
        }

        void AssignTopToAnchorTop(
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
            {
                widget->y = 0.0f;
            }
            else
            {
                widget->y = anchor->y.Get();
            }
        }

        void AssignBottomToAnchorBottom(
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
            {
                widget->y = anchor->height.Get() - widget->height.Get();
            }
            else
            {
                widget->y = anchor->y.Get() + anchor->height.Get() - widget->height.Get();
            }
        }

        void AssignTopToAnchorBottom(
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
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
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
            {
                widget->x = [widget,anchor](){ return (0.5f*(anchor->width.Get() - widget->width.Get())); };
                widget->y = [widget,anchor](){ return (0.5f*(anchor->height.Get() - widget->height.Get())); };
            }
            else
            {
                widget->x = [widget,anchor](){ return (anchor->x.Get() + 0.5f*(anchor->width.Get() - widget->width.Get())); };
                widget->y = [widget,anchor](){ return (anchor->y.Get() + 0.5f*(anchor->height.Get() - widget->height.Get())); };
            }
        }

        // Horizontal
        void BindHCenterToAnchorHCenter(
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
            {
                widget->x = [widget,anchor](){ return (0.5f*(anchor->width.Get() - widget->width.Get())); };
            }
            else
            {
                widget->x = [widget,anchor](){ return (anchor->x.Get() + 0.5f*(anchor->width.Get() - widget->width.Get())); };
            }
        }

        void BindRightToAnchorLeft(
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
            {
                widget->x = [widget,anchor](){ return -1.0f*widget->width.Get(); };
            }
            else
            {
                widget->x = [widget,anchor](){ return (anchor->x.Get() - widget->width.Get()); };
            }
        }

        void BindLeftToAnchorLeft(
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
            {
                widget->x = [widget,anchor](){ return 0.0f; };
            }
            else
            {
                widget->x = [widget,anchor](){ return anchor->x.Get(); };
            }
        }

        void BindLeftToAnchorRight(
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
            {
                widget->x = [widget,anchor](){ return anchor->width.Get(); };
            }
            else
            {
                widget->x = [widget,anchor](){ return (anchor->x.Get() + anchor->width.Get()); };
            }
        }

        void BindRightToAnchorRight(
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
            {
                widget->x = [widget,anchor](){ return (anchor->width.Get() - widget->width.Get()); };
            }
            else
            {
                widget->x = [widget,anchor](){ return (anchor->x.Get() + anchor->width.Get() - widget->width.Get()); };
            }
        }

        // Vertical
        void BindVCenterToAnchorVCenter(
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
            {
                widget->y = [widget,anchor](){ return (0.5f*(anchor->height.Get() - widget->height.Get())); };
            }
            else
            {
                widget->y = [widget,anchor](){ return (anchor->y.Get() + 0.5f*(anchor->height.Get() - widget->height.Get())); };
            }
        }

        void BindBottomToAnchorTop(
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
            {
                widget->y = [widget,anchor](){ return (0.0f - widget->height.Get()); };
            }
            else
            {
                widget->y = [widget,anchor](){ return (anchor->y.Get() - widget->height.Get()); };
            }
        }

        void BindTopToAnchorTop(
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
            {
                widget->y = 0.0f;
            }
            else
            {
                widget->y = anchor->y.Get();
            }
        }

        void BindBottomToAnchorBottom(
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
            {
                widget->y = [widget,anchor](){ return (anchor->height.Get() - widget->height.Get()); };
            }
            else
            {
                widget->y = [widget,anchor](){ return (anchor->y.Get() + anchor->height.Get() - widget->height.Get()); };
            }
        }

        void BindTopToAnchorBottom(
                Widget* widget,
                Widget* anchor)
        {
            EnforceParentOrSibling(widget,anchor);
            if(anchor==widget->GetParent().get())
            {
                widget->y = anchor->height.Get();
            }
            else
            {
                widget->y = [widget,anchor](){ return (anchor->y.Get() + anchor->height.Get()); };
            }
        }
    }
}
