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

#include <raintk/test/RainTkTestContext.hpp>

#include <raintk/RainTkRectangle.hpp>
#include <raintk/RainTkAnimation.hpp>
#include <raintk/RainTkTween.hpp>

namespace raintk
{
    class TestAnimation : public raintk::Animation
    {
    public:
        using base_type = raintk::Animation;

        TestAnimation(ks::Object::Key const &key,
                      Scene* scene,
                      Property<float>* property,
                      Tween::EaseFnPtr ease,
                      Tween::CurveFnPtr curve,
                      uint duration_ms,
                      float from,
                      float to) :
            raintk::Animation(key,scene),
            m_property(property),
            m_ease(ease),
            m_curve(curve),
            m_duration_ms(duration_ms),
            m_from(from),
            m_to(to)
        {}

        void Init(ks::Object::Key const &,
                  shared_ptr<TestAnimation> const &)
        {}

        ~TestAnimation()
        {}

        void start() override
        {
            m_elapsed_ms = 0.0f;
        }

        bool update(float delta_ms) override
        {
            m_elapsed_ms += delta_ms;
            bool completed = false;
            float time = m_elapsed_ms/m_duration_ms;
            if(time > 1.0f)
            {
                completed = true;
                time = 1.0f;
            }

            m_property->Assign(
                        Tween::Eval(
                            (*m_ease)(m_curve,time),
                            m_from,
                            m_to));

            return completed;
        }

        void complete() override
        {}

    private:
        Property<float>* m_property;
        Tween::EaseFnPtr m_ease;
        Tween::CurveFnPtr m_curve;
        float m_duration_ms;
        float m_from;
        float m_to;

        float m_elapsed_ms;
    };
}

using namespace raintk;

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;

    // colors
    std::vector<glm::u8vec3> list_colors {
        glm::u8vec3{255,0,0}, // r
        glm::u8vec3{255,128,0}, // o
        glm::u8vec3{255,255,0}, // y
        glm::u8vec3{0,255,0}, // g
        glm::u8vec3{0,255,255}, // c
        glm::u8vec3{128,0,255}, // i
        glm::u8vec3{255,0,255}, // v
        glm::u8vec3{255,0,127} // p
    };


    // tween curves
    std::vector<Tween::CurveFnPtr> list_tween_curves {
        &Tween::Linear,
        &Tween::Quad,
        &Tween::Cubic,
        &Tween::Quartic,
        &Tween::Quintic,
        &Tween::Sine,
        &Tween::Circle,
        &Tween::Expo
    };


    std::vector<shared_ptr<Animation>> list_animations;

    float y = mm(2);

    for(uint i=1; i < 9; i++)
    {
        auto rect =
                MakeWidget<Rectangle>(
                    c.scene.get(),
                    c.scene->GetRootWidget());

        rect->y = y;
        rect->width = mm(5);
        rect->height = mm(5);
        rect->color = list_colors[i-1];

        y += mm(2+5);

        list_animations.push_back(
                    ks::MakeObject<TestAnimation>(
                        c.scene.get(),
                        &(rect->x),
                        &Tween::EaseInOut,
                        list_tween_curves[i-1],
                        2000,
                        mm(10),
                        mm(100)));

        list_animations.back()->Start();
        list_animations.back()->SetKeepOnComplete(false);
    }

    // Run!
    c.app->Run();

    return 0;
}
