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

#define RAINTK_TEST_OPACITY_HIERARCHY

#include <raintk/test/RainTkTestContext.hpp>
#include <raintk/RainTkRectangle.hpp>
#include <raintk/RainTkText.hpp>
#include <raintk/RainTkDrawSystem.hpp>

using namespace raintk;

shared_ptr<Widget> MakeNode(Scene* scene, shared_ptr<Widget> parent, float opacity)
{
    auto node = MakeWidget<Rectangle>(scene,parent);
    node->width = mm(10);
    node->height = mm(10);
    node->opacity = opacity;

    auto text = MakeWidget<Text>(scene,node);
    text->font = "FiraSansMinimal.ttf";
    text->color = glm::u8vec4(255,255,255,255);
    text->size = mm(5);
    text->text = ks::ToString(opacity);
    text->z = 0.1f;

    return node;
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext context;

    auto root = context.scene->GetRootWidget();
    auto scene = context.scene.get();

    auto b = MakeNode(scene,root,0.5f);
    b->x = mm(30);
    b->y = mm(15);

    auto g = MakeNode(scene,b,0.5f);
    g->x = mm(-15);
    g->y = mm(15);

    auto c = MakeNode(scene,root,0.8f);
    c->x = mm(60);
    c->y = mm(15);

    auto d = MakeWidget<Widget>(scene,c);
    d->x = mm(0);
    d->y = mm(15);

    auto e = MakeNode(scene,d,0.5f);
    e->x = mm(15);
    e->y = mm(15);

    auto f = MakeNode(scene,d,0.5f);
    f->x = mm(-15);
    f->y = mm(15);

    auto h = MakeNode(scene,f,0.5f);
    h->x = mm(0);
    h->y = mm(15);


    scene->signal_after_update.Connect(
                [&](){
                    bool ok = false;
                    ok =
                            (b->m_accumulated_opacity==0.5f) &&
                            (g->m_accumulated_opacity==0.25f) &&
                            (c->m_accumulated_opacity==0.8f) &&
                            (e->m_accumulated_opacity==0.4f) &&
                            (f->m_accumulated_opacity==0.4f) &&
                            (h->m_accumulated_opacity==0.2f);

                    assert(ok);
                });


    // Run!
    context.app->Run();

    return 0;
}
