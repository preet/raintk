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

#ifndef RAINTK_ANIMATION_HPP
#define RAINTK_ANIMATION_HPP

#include <ks/KsObject.hpp>
#include <raintk/RainTkGlobal.hpp>

namespace raintk
{
    class Scene;

    class Animation : public ks::Object
    {
        friend class AnimationSystem;
        friend class TransformSystem;

    public:
        using base_type = ks::Object;

        enum class State
        {
            Stopped,
            Paused,
            Running
        };

        Animation(ks::Object::Key const &key,
                  Scene* scene);

        void Init(ks::Object::Key const &key,
                  shared_ptr<Animation> const &);

        virtual ~Animation();

        void Start();
        void Stop();
        void Pause();
        void Resume();
        void Complete();
        void Update(float delta_ms);

        // Remove animation from scene
        void Remove();

        State GetState() const;

        void SetLoops(uint loops);
        uint GetLoops() const;

        void SetKeepOnComplete(bool keep);
        bool GetKeepOnComplete() const;

    protected:
        virtual void start()=0;

        // Return whether or not the animation has completed
        virtual bool update(float delta_ms)=0;
        virtual void complete()=0;

        State m_state;
        uint m_loops;
        bool m_keep;
        bool m_complete;

    private:
        Scene* m_scene;
        bool m_ready;
    };
}

#endif // RAINTK_ANIMATION_HPP


