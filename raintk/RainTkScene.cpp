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

#include <ks/draw/KsDrawRenderSystem.hpp>
#include <ks/draw/KsDrawDefaultDrawStage.hpp>
#include <ks/gl/KsGLCommands.hpp>
#include <ks/shared/KsImage.hpp>

#include <raintk/RainTkLog.hpp>
#include <raintk/RainTkUnits.hpp>
#include <raintk/RainTkScene.hpp>

#include <raintk/RainTkInputSystem.hpp>
#include <raintk/RainTkAnimationSystem.hpp>
#include <raintk/RainTkTransformSystem.hpp>
#include <raintk/RainTkDrawSystem.hpp>

#include <raintk/RainTkWidget.hpp>
#include <raintk/RainTkMainDrawStage.hpp>

#ifdef RAINTK_TEXT_ENABLED
#include <ks/text/KsTextTextManager.hpp>
#endif

namespace raintk
{
    Scene::Scene(ks::Object::Key const &key,
                 shared_ptr<ks::gui::Application> app,
                 shared_ptr<ks::gui::Window> window) :
        ks::ecs::Scene<SceneKey>(key,app->GetEventLoop()),
        m_app(app),
        m_window(window),
        m_running(false)
    {
        if(window->swap_interval.Get() == 0)
        {
            throw ks::Exception(
                        ks::Exception::ErrorLevel::FATAL,
                        "raintk: Scene: Rendering without "
                        "vsync unsupported!");
        }

        // Current units are unscaled millimeters
        auto list_screens = app->GetScreens();
        m_screen_dpi =
                (list_screens.at(0)->xdpi.Get() +
                 list_screens.at(0)->ydpi.Get())*0.5;

        detail_units::px_per_mm = m_screen_dpi/25.4f;
    }

    void Scene::Init(ks::Object::Key const &,
                     shared_ptr<raintk::Scene> const &)
    {
        // TODO Is posting the task really necessary?
//        // Post the init task to this scene's EventLoop
//        auto init_task =
//                make_shared<ks::Task>(
//                    [this](){
//            this->onInitThis();
//        });

//        this->GetEventLoop()->PostTask(init_task);
//        init_task->Wait();

        this->onInitThis();
    }

    Scene::~Scene()
    {

    }

    InputSystem* Scene::GetInputSystem() const
    {
        return m_input_system.get();
    }

    AnimationSystem* Scene::GetAnimationSystem() const
    {
        return m_animation_system.get();
    }

    TransformSystem* Scene::GetTransformSystem() const
    {
        return m_transform_system.get();
    }

    DrawSystem* Scene::GetDrawSystem() const
    {
        return m_draw_system.get();
    }

    shared_ptr<Widget> const & Scene::GetRootWidget() const
    {
        return m_root_widget;
    }

    Id Scene::GetMainDrawStageId() const
    {
        return m_main_draw_stage_id;
    }

    MainDrawStage* Scene::GetMainDrawStage() const
    {
        return m_main_draw_stage.get();
    }

#ifdef RAINTK_TEXT_ENABLED
    ks::text::TextManager* Scene::GetTextManager() const
    {
        return m_text_manager.get();
    }

    std::map<uint,unique_ptr<TextAtlasData>> const &
    Scene::GetTextAtlasData() const
    {
        return m_lkup_text_atlas_data;
    }

    uint Scene::GetTextAtlasSizePx() const
    {
        return m_atlas_res_px;
    }

    uint Scene::GetTextGlyphSizePx() const
    {
        return m_glyph_res_px;
    }

    uint Scene::GetTextGlyphSDFSizePx() const
    {
        return m_sdf_res_px;
    }
#endif

    void Scene::SetShowDebugText(bool show)
    {
        m_render_system->ShowDebugText(show);
    }

    void Scene::onInitThis()
    {
        shared_ptr<raintk::Scene> this_scene =
                std::static_pointer_cast<raintk::Scene>(
                    shared_from_this());

        auto app = m_app.lock();
        auto window = m_window.lock();

        // Create Systems
        m_render_system = make_unique<RenderSystem>(this);
        m_input_system = make_unique<InputSystem>(this,app.get());
        m_transform_system = make_unique<TransformSystem>(this);
        m_animation_system = make_unique<AnimationSystem>();
        m_draw_system = make_unique<DrawSystem>(this,m_render_system.get());

        // Add the main Draw Stage
        m_main_draw_stage = make_shared<MainDrawStage>();

        m_main_draw_stage_id =
                m_render_system->RegisterDrawStage(
                    m_main_draw_stage);


        // Create the root widget
        m_root_widget =
                MakeWidget<Widget>(
                    Widget::RootWidgetKey{},
                    this_scene);

        m_root_widget->clip = true;


        // Setup camera
        m_camera.SetViewMatrixAsLookAt(
                    glm::vec3(0,0,0),
                    glm::vec3(0,0,-1),
                    glm::vec3(0,1,0));


        // Window dims
        this->onWinSizeChanged(window->size.Get());


        // Text TODO: Allow these params to be changed?
#ifdef RAINTK_TEXT_ENABLED
        m_text_manager =
                make_unique<ks::text::TextManager>(
                    m_atlas_res_px,
                    m_glyph_res_px,
                    m_sdf_res_px);
#endif


        // Application ---> Scene
        app->signal_init.Connect(
                    this_scene,
                    &Scene::onAppInit);

        app->signal_pause.Connect(
                    this_scene,
                    &Scene::onAppPause,
                    ks::ConnectionType::Direct);

        app->signal_resume.Connect(
                    this_scene,
                    &Scene::onAppResume);

        app->signal_quit.Connect(
                    this_scene,
                    &Scene::onAppQuit,
                    ks::ConnectionType::Blocking);

        app->signal_graphics_reset.Connect(
                    this_scene,
                    &Scene::onAppGraphicsReset);

        app->signal_processed_events->Connect(
                    this_scene,
                    &Scene::onAppProcEvents);


        // Scene ---> Application
        m_signal_app_process_events.Connect(
                    app,
                    &ks::gui::Application::ProcessEvents);


        // Window ---> Scene
        window->size.signal_changed.Connect(
                    this_scene,
                    &Scene::onWinSizeChanged);


        // TextManager ---> Scene
#ifdef RAINTK_TEXT_ENABLED
        m_text_manager->signal_new_atlas->Connect(
                    this_scene,
                    &Scene::onNewTextAtlas,
                    ks::ConnectionType::Direct);

        m_text_manager->signal_new_glyph->Connect(
                    this_scene,
                    &Scene::onNewTextGlyph,
                    ks::ConnectionType::Direct);
#endif
    }

    void Scene::onAppInit()
    {
        rtklog.Trace() << "onAppInit";
        m_prev_upd_time = std::chrono::high_resolution_clock::now();
        m_running = true;
        m_signal_app_process_events.Emit();
    }

    void Scene::onAppPause()
    {
        rtklog.Trace() << "onAppPause";
        m_running = false;
    }

    void Scene::onAppResume()
    {
        rtklog.Trace() << "onAppResume";
        m_prev_upd_time = std::chrono::high_resolution_clock::now();
        m_running = true;
        m_signal_app_process_events.Emit();
    }

    void Scene::onAppQuit()
    {
        rtklog.Trace() << "onAppQuit";
        m_running = false;
    }

    void Scene::onAppGraphicsReset()
    {
        rtklog.Trace() << "Scene::onAppGraphicsReset";

        // We expect this signal is sent after rendering
        // has been paused and the graphics context
        // is lost and has already been cleaned up

        // Get a list of all DrawableWidgets (TODO)
        // Destroy all Drawables (TODO)
        // Clear the RenderSystem
        m_render_system->Reset();
        // Reset Renderable Init state (TODO)
        // Recreate Drawables (TODO)
    }

    void Scene::onAppProcEvents(bool)
    {
        if(m_running)
        {
            // Update
            this->onUpdate();


            // Sync
            auto win_ptr = m_window.lock().get();

            auto sync_task =
                    make_shared<ks::Task>(
                        [this,win_ptr](){
                            if(win_ptr->SetContextCurrent())
                            {
                                this->onSync();
                            }
                        });

            win_ptr->GetEventLoop()->PostTask(sync_task);
            sync_task->Wait();


            // Render
            auto render_task =
                    make_shared<ks::Task>(
                        [this,win_ptr](){
                            if(win_ptr->SetContextCurrent())
                            {
                                this->onRender();
                                win_ptr->SwapBuffers();
                                ks::gl::Finish();
                            }
                        });

            win_ptr->GetEventLoop()->PostTask(render_task);


            // Schedule next update, vsync will eventually
            // block this until the next vblank
            m_signal_app_process_events.Emit();
        }
    }

    void Scene::onWinSizeChanged(ks::gui::Window::Size win_size_px)
    {
        auto const width_px = win_size_px.first;
        auto const height_px = win_size_px.second;

        m_window_size_px =
                glm::vec2{
                    width_px,
                    height_px
                };

        m_root_widget->width = px(width_px);
        m_root_widget->height = px(height_px);

        // Update the MainDrawStage view
        m_viewport =
                glm::vec4{
                    0.0f,   // x_px
                    0.0f,   // y_px
                    width_px,
                    height_px
                };

        m_camera.SetProjMatrixAsOrtho(
                    0,                  // left
                    width_px,      // right
                    height_px,     // bottom
                    0,                  // top
                    -100.1,             // near (relative to camera eye)
                    0.1                 // far (relative to camera eye)
                    );
    }

#ifdef RAINTK_TEXT_ENABLED
    void Scene::onNewTextAtlas(uint atlas_index,
                               uint atlas_size_px)
    {
        auto& atlas_data_ptr =
                m_lkup_text_atlas_data[atlas_index];

        if(atlas_data_ptr==nullptr)
        {
            atlas_data_ptr =
                    make_unique<TextAtlasData>();
        }

        TextAtlasData& atlas_data = *atlas_data_ptr;

        atlas_data.texture_set =
                make_shared<ks::draw::TextureSet>();

        atlas_data.texture_set->list_texture_desc.
                emplace_back(
                    make_unique<ks::gl::Texture2D>(
                        ks::gl::Texture2D::Format::LUMINANCE8
                        ),
                        0 // tex unit
                    );

        atlas_data.texture_set_id =
                m_render_system->
                RegisterTextureSet(atlas_data.texture_set);

        atlas_data.atlas_texture =
                atlas_data.texture_set->
                list_texture_desc.back().first.get();

        atlas_data.atlas_texture->SetFilterModes(
                    ks::gl::Texture2D::Filter::Linear,
                    ks::gl::Texture2D::Filter::Linear);

        ks::Image<ks::R8> blank_image(
                    atlas_size_px,
                    atlas_size_px,
                    ks::R8{0});

        atlas_data.atlas_texture->UpdateTexture(
                    ks::gl::Texture2D::Update{
                        ks::gl::Texture2D::Update::ReUpload,
                        glm::u16vec2(0,0),
                        shared_ptr<ks::ImageData>(
                            blank_image.ConvertToImageDataPtr().release()
                        )
                    });
    }

    void Scene::onNewTextGlyph(uint atlas_index,
                               glm::u16vec2 offset,
                               shared_ptr<ks::ImageData> image_data)
    {
        if(m_lkup_text_atlas_data.count(atlas_index)==0)
        {
            throw ks::Exception(
                        ks::Exception::ErrorLevel::ERROR,
                        "Received glyph before atlas created");
        }

        auto& atlas_data = *(m_lkup_text_atlas_data[atlas_index]);

        atlas_data.atlas_texture->UpdateTexture(
                    ks::gl::Texture2D::Update{
                        ks::gl::Texture2D::Update::Defaults,
                        offset,
                        image_data
                    });
    }
#endif

    void Scene::onUpdate()
    {
#ifdef RAINTK_BUILD_DEBUG
        signal_before_update.Emit();
#endif

        TimePoint const curr_upd_time =
                std::chrono::high_resolution_clock::now();

        // Update systems
        m_input_system->Update(m_prev_upd_time,curr_upd_time);
        m_animation_system->Update(m_prev_upd_time,curr_upd_time);
        m_transform_system->Update(m_prev_upd_time,curr_upd_time);
        m_draw_system->Update(m_prev_upd_time,curr_upd_time);
        m_render_system->Update(m_prev_upd_time,curr_upd_time);

        TimePoint const upd_end_time =
                std::chrono::high_resolution_clock::now();

        double update_time_ms =
                ks::CalcDuration<Microseconds>(
                    curr_upd_time,upd_end_time).count()/1000.0;

        std::string update_time_msg =
                "raintk update: " +
                ks::ToStringFormat(update_time_ms,3,7,'0') +
                "ms";

//        rtklog.Trace() << update_time_msg;

        m_render_system->AddCustomDebugText(update_time_msg);

        m_prev_upd_time = curr_upd_time;

#ifdef RAINTK_BUILD_DEBUG
        signal_after_update.Emit();
#endif

    }

    void Scene::onSync()
    {
        m_render_system->Sync();

        m_main_draw_stage->SyncView(
                    m_viewport,m_camera);

        m_main_draw_stage->SyncClipRegions(
                    m_draw_system->GetClipRegions());

        m_main_draw_stage->SyncDrawOrder(
                    m_draw_system->GetOpaqueDrawOrderList(),
                    m_draw_system->GetTransparentDrawOrderList());
    }

    void Scene::onRender()
    {
        m_render_system->Render();
    }
}
