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

#include <raintk/test/RainTkTestContext.hpp>
#include <raintk/RainTkRectangle.hpp>
#include <raintk/RainTkSinglePointArea.hpp>
#include <raintk/RainTkText.hpp>
#include <raintk/RainTkAnimation.hpp>
#include <raintk/RainTkTween.hpp>

using namespace raintk;

class PropAnimation : public Animation
{
public:
    using base_type = raintk::Animation;

    Property<float> from {
        "anim.from",0.0f
    };

    Property<float> to {
        "anim.to",0.0f
    };

    Property<float>* target{nullptr};

    float duration_ms;

    PropAnimation(ks::Object::Key const &key,
                  Scene* scene) :
        Animation(key,scene)
    {

    }

    void Init(ks::Object::Key const &,
              shared_ptr<PropAnimation> const &)
    {

    }

    void start() override
    {
        m_from = 0.0f;
        m_to = 0.0f;
        m_elapsed_ms = 0.0f;

        (*target) = [this](){
            m_from = from.Get();
            m_to = to.Get();

            rtklog.Trace() << "from: " << m_from << ", to: " << m_to;

            return m_from;
        };
    }

    bool update(float delta_ms) override
    {
        m_elapsed_ms += delta_ms;
        bool completed = false;
        float time = m_elapsed_ms/duration_ms;

        if(time > 1.0)
        {
            time = 1.0;
            completed = true;
        }

        (*target) =
                Tween::Eval(
                    Tween::EaseInOut(&Tween::Quad,time),
                    m_from,m_to);

        return completed;
    }

    void complete() override
    {

    }

private:
    float m_from;
    float m_to;
    float m_elapsed_ms;
};

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;

    auto root = c.scene->GetRootWidget();

    auto button_add_bg = MakeWidget<Rectangle>(root,"");

    auto button_add_text = MakeWidget<Text>(button_add_bg,"");
    button_add_text->x = [&](){ return (button_add_bg->width.Get()-button_add_text->width.Get())*0.5f; };
    button_add_text->y = [&](){ return (button_add_bg->height.Get()-button_add_text->height.Get())*0.5f; };
    button_add_text->z = mm(1.0f);
    button_add_text->text = "Add";
    button_add_text->font = "FiraSansMinimal.ttf";
    button_add_text->color = glm::u8vec3(255,255,255);

    button_add_bg->width = [&](){ return button_add_text->width.Get()+mm(5); };
    button_add_bg->height = [&](){ return button_add_text->height.Get()+mm(5); };

    auto button_add_sp = MakeWidget<SinglePointArea>(button_add_bg,"");
    button_add_sp->width = [&](){ return button_add_bg->width.Get(); };
    button_add_sp->height = [&](){ return button_add_bg->height.Get(); };
    button_add_sp->z = mm(2.0f);


    shared_ptr<PropAnimation> animation;
    shared_ptr<Rectangle> r0;

    button_add_sp->signal_clicked.Connect(
                [&](){
                    button_add_text->text = "Hello World";

                    // At this point the text width and height
                    // are incorrect and an update is pending

                    r0 = MakeWidget<Rectangle>(root,"r0");
                    r0->x = 0;
                    r0->y = mm(20);
                    r0->width = [&](){ return button_add_text->width.Get()+mm(5); };
                    r0->height = [&](){ return button_add_text->height.Get()+mm(5); };

                    // Lets animate the width from 0

                    // but animations are run before transforms
                    // the first Animation::Update should be delayed by one frame

                    animation = ks::MakeObject<PropAnimation>(c.scene.get());
                    animation->target = &(r0->width);
                    animation->from = 0.0f;
                    animation->to = [&](){ return button_add_text->width.Get() + mm(5); };
                    animation->duration_ms = 300.0f;
                    animation->Start();

                    // Since the start of the animation is delayed by one frame,
                    // the width and height of the text and corresponding button
                    // should be updated by the time update() is called on the
                    // animation. The width of r0 should thus be the text width
                    // of "Hello World" plus the specified margin (confirm visually)
                });

    // Run!
    c.app->Run();

    return 0;
}
