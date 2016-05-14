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

#ifndef RAINTK_COLOR_CONV_HPP
#define RAINTK_COLOR_CONV_HPP

#include <glm/glm.hpp>
#include <ks/KsException.hpp>

namespace raintk
{
    // =========================================================== //

    class ColorInvalid : public ks::Exception
    {
    public:
        ColorInvalid();
        ~ColorInvalid() = default;
    };

    // =========================================================== //

    namespace Color
    {
        // Expects a string that starts with a hash '#',
        // followed by 6 or 8 hex characters 0-9,a-f,A-F
        // String will be interpreted as the color channels
        // R,G,B,A from left to right for every two chars
        // ex: #0A0B0CFF: R=10, G=11, B=12, A=255
        glm::u8vec4 ConvHexToVec4(std::string hex_str);
    }

    // =========================================================== //
}

#endif // RAINTK_COLOR_CONV_HPP


