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

#ifndef RAINTK_CONFIG_HPP
#define RAINTK_CONFIG_HPP

// platforms
#if defined(__ANDROID__)
    #define RAINTK_ENV_ANDROID 1
#elif defined(__linux__)
    #define RAINTK_ENV_LINUX 1
#elif defined(__APPLE__) && defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE == 1
        #define RAINTK_ENV_APPLE_IOS 1
    #elif TARGET_OS_MAC == 1
        #define RAINTK_ENV_APPLE_OSX 1
    #endif
#endif

#endif // RAINTK_CONFIG_HPP
