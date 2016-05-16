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

#ifndef RAINTK_TRANSFORM_SYSTEM_HPP
#define RAINTK_TRANSFORM_SYSTEM_HPP

#include <ks/draw/KsDrawSystem.hpp>
#include <raintk/RainTkGlobal.hpp>
#include <raintk/RainTkComponents.hpp>

namespace raintk
{
    class Scene;
    class DrawableWidget;

    class TransformSystem : public ks::draw::System
    {
    public:
        TransformSystem(Scene* scene);

        ~TransformSystem();

        std::string GetDesc() const override;

        TransformDataComponentList*
        GetTransformDataComponentList() const;

        void Update(TimePoint const &prev_time,
                    TimePoint const &curr_time) override;

    private:
        void updateLayout();
        void updateTransforms();
        void updateAnimations();

        static void updateWidgetTransforms(
                Widget* widget,
                UpdateDataComponentList* cmlist_upd_data,
                TransformDataComponentList* cmlist_xf_data);

        static void updateWidgetClips(
                Widget* widget,
                UpdateDataComponentList* cmlist_upd_data,
                TransformDataComponentList* cmlist_xf_data);

        static void updateWidgetOpacities(
                Widget* widget,
                float opacity);

        Scene* const m_scene;
        UpdateDataComponentList* m_cmlist_upd_data;
        TransformDataComponentList* m_cmlist_xf_data;
    };
}

#endif // RAINTK_TRANSFORM_SYSTEM_HPP



