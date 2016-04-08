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

#ifndef RAINTK_GLOBAL_HPP
#define RAINTK_GLOBAL_HPP

#include <ks/KsGlobal.hpp>

namespace raintk
{
    // Convenience typedefs
    using uint = ks::uint;
    using u8 = ks::u8;
    using u16 = ks::u16;
    using u32 = ks::u32;
    using u64 = ks::u64;

    using sint = ks::sint;
    using s8 = ks::s8;
    using s16 = ks::s16;
    using s32 = ks::s32;
    using s64 = ks::s64;

    using Id = ks::Id;

    using std::shared_ptr;
    using std::unique_ptr;
    using std::weak_ptr;

    // Note: each of the predefined duration types
    // covers a range of at least ±292 years
    using Microseconds = ks::Microseconds;
    using Milliseconds = ks::Milliseconds;
    using Seconds = ks::Seconds;
    using Minutes = ks::Minutes;
    using TimePoint = ks::TimePoint;

    using std::make_shared;
    using ks::make_unique;
}

#endif // RAINTK_GLOBAL_HPP
