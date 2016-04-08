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

#ifndef RAINTK_ANIMATION_SYSTEM_HPP
#define RAINTK_ANIMATION_SYSTEM_HPP

#include <vector>
#include <ks/KsException.hpp>
#include <ks/draw/KsDrawSystem.hpp>
#include <raintk/RainTkGlobal.hpp>

namespace raintk
{
    class Animation;

    class AnimationNotFound : public ks::Exception
    {
    public:
        AnimationNotFound();
        ~AnimationNotFound() = default;
    };

    class AnimationAlreadyExists : public ks::Exception
    {
    public:
        AnimationAlreadyExists();
        ~AnimationAlreadyExists() = default;
    };

    class AnimationSystem : public ks::draw::System
    {
    public:
        AnimationSystem();

        ~AnimationSystem();

        std::string GetDesc() const override;

        void Update(TimePoint const &prev_time,
                    TimePoint const &curr_time) override;

        void AddAnimation(Animation* animation);
        void RemoveAnimation(Animation* animation);

        std::vector<Animation*> const & GetListAnimations() const;

    private:
        std::vector<Animation*> m_list_animations;
    };
}

#endif // RAINTK_ANIMATION_SYSTEM_HPP



