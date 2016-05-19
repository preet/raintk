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

#ifndef RAINTK_SCENE_HPP
#define RAINTK_SCENE_HPP

#include <ks/gui/KsGuiApplication.hpp>
#include <ks/ecs/KsEcs.hpp>
#include <ks/gl/KsGLCamera.hpp>

#include <raintk/RainTkSceneKey.hpp>
#include <raintk/RainTkDrawKey.hpp>
#include <raintk/RainTkComponents.hpp>

// For debugging
#define RAINTK_TEXT_ENABLED

#ifdef RAINTK_TEXT_ENABLED
    #include <ks/text/KsTextTextManager.hpp>
#endif

namespace ks
{
    struct ImageData;

    namespace draw
    {
        struct TextureSet;
    }
}

namespace raintk
{
    class Widget;

    class InputSystem;
    class AnimationSystem;
    class TransformSystem;
    class DrawSystem;

    class MainDrawStage;

    class Scene : public ks::ecs::Scene<SceneKey>
    {
    public:
        using base_type = ks::ecs::Scene<SceneKey>;

        Scene(ks::Object::Key const &key,
              shared_ptr<ks::gui::Application> app,
              shared_ptr<ks::gui::Window> window);

        void Init(ks::Object::Key const &,
                  shared_ptr<raintk::Scene> const &);

        ~Scene();


        InputSystem* GetInputSystem() const;
        AnimationSystem* GetAnimationSystem() const;
        TransformSystem* GetTransformSystem() const;
        DrawSystem* GetDrawSystem() const;

        shared_ptr<Widget> const &GetRootWidget() const;
        Id GetMainDrawStageId() const;
        MainDrawStage* GetMainDrawStage() const; // debug

#ifdef RAINTK_TEXT_ENABLED
        ks::text::TextManager* GetTextManager() const;
        std::map<uint,unique_ptr<TextAtlasData>> const &
        GetTextAtlasData() const;
        uint GetTextAtlasSizePx() const;
        uint GetTextGlyphSizePx() const;
        uint GetTextGlyphSDFSizePx() const;
#endif

        template<typename... Args>
        bool GetEntityHasComponents(Id ent_id) const
        {
            auto mask = this->GetComponentMask<Args...>();
            return ((this->GetEntityList()[ent_id].mask & mask) == mask);
        }

        void SetShowDebugText(bool show);


#ifdef RAINTK_BUILD_DEBUG
        ks::Signal<> signal_before_update;
        ks::Signal<> signal_after_update;
#endif

    private:
        void onInitThis();
        void onAppInit();
        void onAppPause();
        void onAppResume();
        void onAppQuit();
        void onAppGraphicsReset();
        void onAppProcEvents(bool);

        void onWinSizeChanged(ks::gui::Window::Size);

#ifdef RAINTK_TEXT_ENABLED
        void onNewTextAtlas(uint atlas_index,
                            uint atlas_size_px);

        void onNewTextGlyph(uint atlas_index,
                            glm::u16vec2 offset,
                            shared_ptr<ks::ImageData> image_data);
#endif

        void onUpdate();
        void onSync();
        void onRender();

        weak_ptr<ks::gui::Application> m_app;
        weak_ptr<ks::gui::Window> m_window;

        // Display and window
        float m_screen_dpi;
        glm::vec2 m_window_size_px;

        // Systems
        unique_ptr<InputSystem> m_input_system;
        unique_ptr<AnimationSystem> m_animation_system;
        unique_ptr<TransformSystem> m_transform_system;
        unique_ptr<DrawSystem> m_draw_system;
        unique_ptr<RenderSystem> m_render_system;

        // App update loop
        ks::Signal<> m_signal_app_process_events;
        std::atomic<bool> m_running;
        std::atomic<bool> m_sync_pending;
        TimePoint m_prev_upd_time;
        shared_ptr<ks::CallbackTimer> m_idle_timer;

        // Root widget
        shared_ptr<Widget> m_root_widget;
        shared_ptr<Widget> m_focus_widget;

        // Draw stage
        Id m_main_draw_stage_id;
        shared_ptr<MainDrawStage> m_main_draw_stage;
        glm::vec4 m_viewport;
        ks::gl::Camera<float> m_camera;


#ifdef RAINTK_TEXT_ENABLED
        // Text
        uint const m_atlas_res_px{1024};
        uint const m_glyph_res_px{32};
        uint const m_sdf_res_px{4};
        unique_ptr<ks::text::TextManager> m_text_manager;
        std::map<uint,unique_ptr<TextAtlasData>> m_lkup_text_atlas_data;
#endif
    };

    // ============================================================= //

    //    // == Simulate == //

    //    // Use the elapsed time between calls to this function
    //    // to accumulate lag that will be simulated in steps
    //    // determined by @m_sim_dt_us
    //    time_point const curr_upd_time =
    //            std::chrono::high_resolution_clock::now();

    //    auto const elapsed_us =
    //            std::chrono::duration_cast<
    //            std::chrono::microseconds>(
    //                curr_upd_time-m_prev_upd_time);

    //    m_sim_lag_us += elapsed_us;
    //    m_prev_upd_time = curr_upd_time;

    //    while(m_sim_lag_us >= m_sim_dt_us) {
    //        // do work for:
    //        // t0=m_sim_time_us,
    //        // t1=(m_sim_time_us + m_sim_dt_us)

    //        // (do work here)

    //        m_sim_lag_us -= m_sim_dt_us;
    //        m_sim_time_us += m_sim_dt_us;
    //    }

    // ============================================================= //
}

#endif // RAINTK_SCENE_HPP
