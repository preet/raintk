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

#include <algorithm>

#include <raintk/RainTkAnimation.hpp>
#include <raintk/RainTkAnimationSystem.hpp>

namespace raintk
{
    AnimationNotFound::AnimationNotFound() :
        ks::Exception(ks::Exception::ErrorLevel::FATAL,"",true)
    {}

    AnimationAlreadyExists::AnimationAlreadyExists() :
        ks::Exception(ks::Exception::ErrorLevel::FATAL,"",true)
    {}

    AnimationSystem::AnimationSystem()
    {

    }

    AnimationSystem::~AnimationSystem()
    {

    }

    std::string AnimationSystem::GetDesc() const
    {
        return "AnimationSystem";
    }

    void AnimationSystem::Update(TimePoint const &prev_time,
                                 TimePoint const &curr_time)
    {
        float const elapsed_ms =
                std::chrono::duration_cast<
                    std::chrono::microseconds>(
                        curr_time-prev_time).count()/1000.0;

        for(Animation* animation : m_list_animations)
        {
            if(animation->m_ready)
            {
                animation->Update(elapsed_ms);
            }
        }
    }

    void AnimationSystem::AddAnimation(Animation* animation)
    {
        auto it =
                std::find(
                    m_list_animations.begin(),
                    m_list_animations.end(),
                    animation);

        if(it != m_list_animations.end())
        {
            throw AnimationAlreadyExists();
        }

        m_list_animations.push_back(animation);
    }

    void AnimationSystem::RemoveAnimation(Animation* animation)
    {
        auto it =
                std::find(
                    m_list_animations.begin(),
                    m_list_animations.end(),
                    animation);

        if(it == m_list_animations.end())
        {
            throw AnimationNotFound();
        }

        m_list_animations.erase(it);
    }

    std::vector<Animation*> const & AnimationSystem::GetListAnimations() const
    {
        return m_list_animations;
    }
}
