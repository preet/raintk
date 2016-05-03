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

#ifndef RAINTK_INPUT_SYSTEM_HPP
#define RAINTK_INPUT_SYSTEM_HPP

#include <ks/draw/KsDrawSystem.hpp>
#include <raintk/RainTkComponents.hpp>

namespace ks
{
    namespace gui
    {
        class Application;
    }
}

namespace raintk
{
    class Scene;
    class InputRecorder;
    class InputListener;
    class InputReplay;

    class InputSystem : public ks::draw::System
    {
    public:
        InputSystem(Scene* scene,
                    ks::gui::Application* app);

        ~InputSystem();

        std::string GetDesc() const override;

        void Update(TimePoint const &prev_time,
                    TimePoint const &curr_time) override;

        InputDataComponentList*
        GetInputDataComponentList() const;

        // * Returns the most recently obtained list of InputAreas
        //   sorted by their (increasing) world depth value
        // * The list is updated every frame during InputSystem::Update
        std::vector<std::pair<float,InputArea*>> const &
        GetInputAreasByDepth() const;

        void StartInputRecording(std::string const &file_name);
        void StopInputRecording();

        void StartInputPlayback(std::string const &file_name);
        void StopInputPlayback();

    private:
        Scene* const m_scene;
        ks::gui::Application* const m_app;
        uint m_inputable_mask;
        InputDataComponentList* m_cmlist_input_data;

        shared_ptr<InputRecorder> m_input_recorder;
        shared_ptr<InputListener> m_input_listener;
        shared_ptr<InputReplay> m_input_replay;

        std::vector<std::pair<float,InputArea*>> m_list_input_areas_by_depth;
    };
}

#endif // RAINTK_INPUT_SYSTEM_HPP




