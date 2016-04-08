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

#ifndef RAINTK_UNITS_HPP
#define RAINTK_UNITS_HPP

#include <functional>
#include <raintk/RainTkProperty.hpp>

namespace raintk
{
    class Scene;

    // Length
    namespace detail_units
    {       
        extern float px_per_mm;
    }

    float const k_pi = 3.14159265358979323846f;

    float px(float x);
    float mm(float x);
    float cm(float x);
    float in(float x);
    float pt(float x);
    float deg(float x);
    float rad(float x);

}

#endif // RAINTK_UNITS_HPP
