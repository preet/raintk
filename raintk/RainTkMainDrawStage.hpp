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

#ifndef RAINTK_MAIN_DRAW_STAGE_HPP
#define RAINTK_MAIN_DRAW_STAGE_HPP

#include <ks/gl/KsGLCamera.hpp>
#include <ks/draw/KsDrawDrawStage.hpp>
#include <raintk/RainTkDrawKey.hpp>
#include <raintk/RainTkComponents.hpp>

namespace raintk
{
    class MainDrawStage : public ks::draw::DrawStage<DrawKey>
    {
        using DrawCall = ks::draw::DrawCall<DrawKey>;

    public:
        MainDrawStage() = default;

        ~MainDrawStage() = default;

        // Functions with names beginning with 'Sync'
        // must only be called when rendering is blocked
        void SyncView(glm::vec4 const &viewport,
                      ks::gl::Camera<float> const &camera);

        void SyncClipRegions(std::vector<BoundingBox> const &list_clip_regions);

        void SyncDrawOrder(std::vector<Id> const &list_opq_draw_order,
                           std::vector<Id> const &list_xpr_draw_order);

        // Render must be called from the render thread
        void Render(ks::draw::DrawParams<DrawKey>& p);

        void Reset();

    private:
        void setupState(glm::vec4 const &viewport,
                        ks::gl::Camera<float> const &camera,
                        ks::draw::DrawParams<DrawKey>& p,
                        DrawKey& prev_key,
                        DrawKey const curr_key);

        void issueDrawCall(ks::gl::ShaderProgram* shader,
                           DrawCall &draw_call);

        glm::vec4 m_viewport;
        ks::gl::Camera<float> m_camera;
        std::vector<BoundingBox> m_list_clip_regions;
        std::vector<Id> m_list_opq_draw_order;
        std::vector<Id> m_list_xpr_draw_order;
    };
}

#endif // RAINTK_MAIN_DRAW_STAGE_HPP
