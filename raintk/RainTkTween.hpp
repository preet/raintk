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

#ifndef RAINTK_TWEEN_HPP
#define RAINTK_TWEEN_HPP

#include <vector>
#include <cmath>
#include <raintk/RainTkGlobal.hpp>

namespace raintk
{
    namespace Tween
    {
        using TweenFnPtr = float(*)(float);
        using CurveFnPtr = float(*)(float);
        using EaseFnPtr = float(*)(TweenFnPtr,float);

        // Tweening functions based on Robert Penner's Easing Functions
        // (http://robertpenner.com/easing/)

        // Usage example:
        // Find the value with a scaling of 0.75 between
        // 5 and 10 using a Quadratic curve and InOut easing
        // float val = Tween::Eval(Tween::EaseInOut(&Tween::Quad,0.75f),5,10);

        // Easing in: Not-moving to moving
        // Easing out: Moving to not-moving

        // Curve types (default is easing in):
        float Linear(float t);
        float Quad(float t);
        float Cubic(float t);
        float Quartic(float t);
        float Quintic(float t);
        float Sine(float t);
        float Circle(float t);
        float Expo(float t);


        // Easing functions
        float EaseIn(TweenFnPtr fn, float t);
        float EaseOut(TweenFnPtr fn, float t);
        float EaseInOut(TweenFnPtr fn, float t);


        // Convenience Evaluation function

        // k: The interpolation factor [0,1]
        // initial: Initial value
        // final: final value
        float Eval(float k, float initial, float final);

        // delta: The difference: (final value - intial value)
        float EvalDelta(float k, float initial, float delta);
    }




//    class Tween
//    {
//    public:
//        enum class Curve : u8
//        {
//            Linear = 0,
//            Quad,
//            Cubic,
//            Quartic,
//            Quintic,
//            Sine,
//            Circle,
//            Expo
//        };

//        enum class Easing : u8
//        {
//            In = 0,
//            Out,
//            InOut
//        };

//        Tween(Curve curve=Curve::Linear,
//              Easing easing=Easing::In,
//              float duration_ms=1000,
//              std::initializer_list<float> list_init_values={},
//              std::initializer_list<float> list_final_values={});

//        // Returns whether or not the current elapsed time
//        // (after adding @delta_ms) exceeds the duration
//        bool Update(float delta_ms);

//        // Resets the current elapsed time to 0
//        void Reset();

//        std::vector<float> const & GetValues() const;

//    private:
//        Curve m_curve;
//        Easing m_easing;
//        float m_duration_ms;
//        float m_elapsed_ms;

//        // TODO: small_vecs?
//        std::vector<float> m_list_init_values;
//        std::vector<float> m_list_delta_values;
//        std::vector<float> m_list_curr_values;
//    };
}

#endif // RAINTK_TWEEN_HPP

