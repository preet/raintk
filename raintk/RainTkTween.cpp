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

#include <raintk/RainTkTween.hpp>
#include <raintk/RainTkLog.hpp>
#include <cmath>

namespace raintk
{
    namespace Tween
    {
        double const k_pi_div2 = 1.57079632679489661923;

        // Tweening functions based on Robert Penner's Easing Functions
        // (http://robertpenner.com/easing/)

        // Easing in: Not-moving to moving
        // Easing out: Moving to not-moving

        // Curve types (default is easing in):

        float Linear(float t)
        {
            return t;
        }

        float Quad(float t)
        {
            return t*t;
        }

        float Cubic(float t)
        {
            return t*t*t;
        }

        float Quartic(float t)
        {
            return t*t*t*t;
        }

        float Quintic(float t)
        {
            return t*t*t*t*t;
        }

        float Sine(float t)
        {
            return 1.0-cos(t*k_pi_div2);
        }

        float Circle(float t)
        {
            return 1.0-sqrt(1.0-(t*t));
        }

        float Expo(float t)
        {
            return pow(2,10*(t-1.0));
        }


        // Easing functions

        float EaseIn(TweenFnPtr fn, float t)
        {
            return fn(t);
        }

        float EaseOut(TweenFnPtr fn, float t)
        {
            // Flip the default ease in function (fn) along x and y
            return (1.0 - fn(1.0-t));
        }

        float EaseInOut(TweenFnPtr fn, float t)
        {
            // Create a piece wise function composed of scaled down
            // ease in (t < 0.5) and ease out functions (t >= 0.5)
            if(t < 0.5)
            {
                return 0.5*EaseIn(fn,t*2);
            }
            else
            {
                return (0.5*EaseOut(fn,(t-0.5)*2)) + 0.5;
            }
        }


        // Helpers to evaluate interpolation

        float Eval(float k, float initial, float final)
        {
            return (k*(final-initial))+initial;
        }

        float EvalDelta(float k, float initial, float delta)
        {
            return (k*delta)+initial;
        }
    }
}
