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

#include <raintk/RainTkScene.hpp>
#include <raintk/RainTkWidget.hpp>
#include <raintk/RainTkAnimation.hpp>
#include <raintk/RainTkAnimationSystem.hpp>

#include <raintk/RainTkLog.hpp>

namespace raintk
{
    Animation::Animation(ks::Object::Key const &key,
                         Scene* scene) :
        ks::Object(key,scene->GetEventLoop()),
        m_state(State::Stopped),
        m_loops(1),
        m_keep(false),
        m_ready(false),
        m_complete(false),
        m_scene(scene)
    {}

    void Animation::Init(ks::Object::Key const &,
                         shared_ptr<Animation> const &)
    {
        // Add this animation to the AnimationSystem so it
        // receives updates
        m_scene->GetAnimationSystem()->AddAnimation(this);
    }

    Animation::~Animation()
    {
        Remove();
    }

    void Animation::Start()
    {
        if(m_state == State::Stopped)
        {
            m_ready = false;
            m_complete = false;
            this->start();

            m_state = State::Running;
        }
    }

    void Animation::Stop()
    {
        m_state = State::Stopped;
    }

    void Animation::Pause()
    {
        if(m_state == State::Running)
        {
            m_state = State::Paused;
        }
    }

    void Animation::Resume()
    {
        if(m_state == State::Paused)
        {
            m_state = State::Running;
        }
    }

    void Animation::Complete()
    {
        this->complete();
        m_complete = true;
        m_state = State::Stopped;

        if(!m_keep)
        {
            Remove();
        }
    }

    void Animation::Update(float delta_ms)
    {
        if(m_state == State::Running)
        {
            if(this->update(delta_ms))
            {
                m_state = State::Stopped;
                m_complete = true;

                if(!m_keep)
                {
                    Remove();
                }
            }
        }
    }

    void Animation::Remove()
    {
        if(m_scene==nullptr)
        {
            return;
        }

        // Tell the AnimationManager to remove this Animation
        m_scene->GetAnimationSystem()->RemoveAnimation(this);

        // Mark scene as null to indicate we've already
        // removed this Animation
        m_scene = nullptr;
    }

    Animation::State Animation::GetState() const
    {
        return m_state;
    }

    void Animation::SetLoops(uint loops)
    {
        m_loops = loops;
    }

    uint Animation::GetLoops() const
    {
        return m_loops;
    }

    void Animation::SetKeepOnComplete(bool keep)
    {
        m_keep = keep;
    }

    bool Animation::GetKeepOnComplete() const
    {
        return m_keep;
    }
}
