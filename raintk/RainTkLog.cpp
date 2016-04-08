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

#include <raintk/RainTkConfig.hpp>
#include <raintk/RainTkGlobal.hpp>
#include <raintk/RainTkLog.hpp>

namespace raintk
{
    ks::Log::Logger rtklog(
            true, // thread-safe
#ifdef RAINTK_ENV_ANDROID
            make_shared<ks::Log::SinkToLogCat>(),
#else
            make_shared<ks::Log::SinkToStdOut>(), // log to stdout by default
#endif
            {{ // array init-list
               { new ks::Log::FBRunTimeMs(), new ks::Log::FBCustomStr(": TRACE: RTK: ") },
               { new ks::Log::FBRunTimeMs(), new ks::Log::FBCustomStr(": DEBUG: RTK: ") },
               { new ks::Log::FBRunTimeMs(), new ks::Log::FBCustomStr(": INFO:  RTK: ") },
               { new ks::Log::FBRunTimeMs(), new ks::Log::FBCustomStr(": WARN:  RTK: ") },
               { new ks::Log::FBRunTimeMs(), new ks::Log::FBCustomStr(": ERROR: RTK: ") },
               { new ks::Log::FBRunTimeMs(), new ks::Log::FBCustomStr(": FATAL: RTK: ") }
             }});

//            {{ // array init-list
//               { new ks::Log::FBCustomStr("TRACE: RAINTK: ") },
//               { new ks::Log::FBCustomStr("DEBUG: RAINTK: ") },
//               { new ks::Log::FBCustomStr("INFO:  RAINTK: ") },
//               { new ks::Log::FBCustomStr("WARN:  RAINTK: ") },
//               { new ks::Log::FBCustomStr("ERROR: RAINTK: ") },
//               { new ks::Log::FBCustomStr("FATAL: RAINTK: ") }
//             }});
}
