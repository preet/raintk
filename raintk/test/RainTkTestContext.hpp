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

#ifndef RAINTK_TEST_CONTEXT_HPP
#define RAINTK_TEST_CONTEXT_HPP

#include <ks/gui/KsGuiApplication.hpp>
#include <raintk/RainTkGlobal.hpp>
#include <raintk/RainTkUnits.hpp>
#include <raintk/RainTkScene.hpp>
#include <raintk/RainTkLog.hpp>

namespace raintk
{
    namespace test
    {
        extern std::vector<unsigned char> const fira_sans_minimal_ttf;
    }
}

namespace
{
    struct TestContext
    {
        TestContext(uint window_width=600,
                    uint window_height=360)
        {
            app = ks::MakeObject<ks::gui::Application>();

            // Create the render thread - uncomment for
            // threaded rendering
//            render_evl = ks::make_shared<ks::EventLoop>();
//            render_thread = ks::EventLoop::LaunchInThread(render_evl);

            // Create window
            ks::gui::Window::Attributes win_attribs;
            ks::gui::Window::Properties win_props;
//            win_attribs.samples = 2;
//            win_attribs.samples = 4;
            win_props.width = window_width;
            win_props.height = window_height;
            win_props.swap_interval = 1;

            window =
                    app->CreateWindow(
//                        render_evl,
                        app->GetEventLoop(),
                        win_attribs,
                        win_props);

            // Create Scene
            scene =
                    ks::MakeObject<raintk::Scene>(
                        app,
                        window);

            // Add font we use for testing
            scene->GetTextManager()->AddFont(
                        "FiraSansMinimal.ttf",
                        ks::make_unique<std::vector<ks::u8>>(
                            raintk::test::fira_sans_minimal_ttf));
        }

        ~TestContext()
        {
            // Uncomment when using threaded rendering
            // Stop threads
//            ks::EventLoop::RemoveFromThread(
//                        render_evl,render_thread,true);
        }

        ks::shared_ptr<ks::EventLoop> render_evl;
        std::thread render_thread;

        ks::shared_ptr<ks::gui::Application> app;
        ks::shared_ptr<ks::gui::Window> window;
        ks::shared_ptr<raintk::Scene> scene;
    };
}

#endif // RAINTK_TEST_CONTEXT_HPP
