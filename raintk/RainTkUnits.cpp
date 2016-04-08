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

#include <raintk/RainTkUnits.hpp>
#include <raintk/RainTkLog.hpp>

namespace raintk
{
    namespace detail_units
    {
        float px_per_mm{10.0};
    }

    float px(float x)
    {
        return x;
    }

    float mm(float x)
    {
        return x*detail_units::px_per_mm;
    }

    float cm(float x)
    {
        return x*detail_units::px_per_mm * (0.1f);
    }

    float in(float x)
    {
        return x*detail_units::px_per_mm * (25.4f);
    }

    float pt(float x)
    {
        return x*detail_units::px_per_mm * (25.4f/72.0f);
    }

    float deg(float x)
    {
        return x*k_pi/180.0f;
    }

    float rad(float x)
    {
        return x;
    }
}
