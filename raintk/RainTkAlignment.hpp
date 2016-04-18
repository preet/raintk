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
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);

        // Horizontal
        void AssignHCenterToAnchorHCenter(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);

        void AssignRightToAnchorLeft(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);

        void AssignLeftToAnchorLeft(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);

        void AssignLeftToAnchorRight(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);

        void AssignRightToAnchorRight(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);

        // Vertical
        void AssignVCenterToAnchorVCenter(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);

        void AssignBottomToAnchorTop(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);

        void AssignTopToAnchorTop(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);

        void AssignBottomToAnchorBottom(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);

        void AssignTopToAnchorBottom(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);


        // Bindings

        void BindCenterToAnchorCenter(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);

        // Horizontal
        void BindHCenterToAnchorHCenter(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);

        void BindRightToAnchorLeft(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);

        void BindLeftToAnchorLeft(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);

        void BindLeftToAnchorRight(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);

        void BindRightToAnchorRight(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);

        // Vertical
        void BindVCenterToAnchorVCenter(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);

        void BindBottomToAnchorTop(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);

        void BindTopToAnchorTop(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);

        void BindBottomToAnchorBottom(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);

        void BindTopToAnchorBottom(
                shared_ptr<Widget> const &widget,
                shared_ptr<Widget> const &anchor);
    }

    // =========================================================== //
}

#endif // RAINTK_ALIGNMENT_HPP


