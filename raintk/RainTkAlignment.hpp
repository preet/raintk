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

#ifndef RAINTK_ALIGNMENT_HPP
#define RAINTK_ALIGNMENT_HPP

#include <ks/KsException.hpp>

namespace raintk
{
    class Widget;

    // =========================================================== //

    class WidgetAlignmentInvalid : public ks::Exception
    {
    public:
        WidgetAlignmentInvalid(std::string msg);
        ~WidgetAlignmentInvalid() = default;
    };

    // =========================================================== //

    namespace Align
    {
        // Assignments

        void AssignCenterToAnchorCenter(
                Widget* widget,
                Widget* anchor);

        // Horizontal
        void AssignHCenterToAnchorHCenter(
                Widget* widget,
                Widget* anchor);

        void AssignRightToAnchorLeft(
                Widget* widget,
                Widget* anchor);

        void AssignLeftToAnchorLeft(
                Widget* widget,
                Widget* anchor);

        void AssignLeftToAnchorRight(
                Widget* widget,
                Widget* anchor);

        void AssignRightToAnchorRight(
                Widget* widget,
                Widget* anchor);

        // Vertical
        void AssignVCenterToAnchorVCenter(
                Widget* widget,
                Widget* anchor);

        void AssignBottomToAnchorTop(
                Widget* widget,
                Widget* anchor);

        void AssignTopToAnchorTop(
                Widget* widget,
                Widget* anchor);

        void AssignBottomToAnchorBottom(
                Widget* widget,
                Widget* anchor);

        void AssignTopToAnchorBottom(
                Widget* widget,
                Widget* anchor);


        // Bindings

        void BindCenterToAnchorCenter(
                Widget* widget,
                Widget* anchor);

        // Horizontal
        void BindHCenterToAnchorHCenter(
                Widget* widget,
                Widget* anchor);

        void BindRightToAnchorLeft(
                Widget* widget,
                Widget* anchor);

        void BindLeftToAnchorLeft(
                Widget* widget,
                Widget* anchor);

        void BindLeftToAnchorRight(
                Widget* widget,
                Widget* anchor);

        void BindRightToAnchorRight(
                Widget* widget,
                Widget* anchor);

        // Vertical
        void BindVCenterToAnchorVCenter(
                Widget* widget,
                Widget* anchor);

        void BindBottomToAnchorTop(
                Widget* widget,
                Widget* anchor);

        void BindTopToAnchorTop(
                Widget* widget,
                Widget* anchor);

        void BindBottomToAnchorBottom(
                Widget* widget,
                Widget* anchor);

        void BindTopToAnchorBottom(
                Widget* widget,
                Widget* anchor);
    }

    // =========================================================== //
}

#endif // RAINTK_ALIGNMENT_HPP


