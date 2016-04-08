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

using namespace raintk;

shared_ptr<Widget> CreateStripes(shared_ptr<Widget> parent,
                                 float width_mm,
                                 glm::vec3 color)
{
    float height_mm = mm(7);

    auto p = MakeWidget<Widget>(parent,"p");
    p->height = 5*height_mm;
    p->width = width_mm;

    auto r0 = MakeWidget<Rectangle>(p,"r0");
    r0->height = height_mm;
    r0->width = width_mm;
    r0->color = color;

    auto r1 = MakeWidget<Rectangle>(p,"r1");
    r1->height = height_mm;
    r1->width = width_mm;
    r1->color = color;
    r1->y = 2*height_mm;

    auto r2 = MakeWidget<Rectangle>(p,"r2");
    r2->height = height_mm;
    r2->width = width_mm;
    r2->color = color;
    r2->y = 4*height_mm;

    return p;
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;


    auto root_widget_ptr =
            c.scene->GetRootWidget().get();


    // Create the left view
    auto left_view =
            MakeWidget<Widget>(
                c.scene->GetRootWidget(),
                "left_view");

    left_view->width =
            [root_widget_ptr]() {
                return 0.5*root_widget_ptr->width.Get();
            };

    left_view->height =
            [root_widget_ptr]() {
                return root_widget_ptr->height.Get();
            };

    left_view->z = mm(5);

    left_view->clip = true;


    // Create two right views
    auto right_view0 =
            MakeWidget<Rectangle>(
                c.scene->GetRootWidget(),
                "right_view0");

    right_view0->x =
            [root_widget_ptr]() {
                return 0.5*root_widget_ptr->width.Get();
            };

    right_view0->width =
            [root_widget_ptr]() {
                return 0.5*root_widget_ptr->width.Get();
            };

    right_view0->height =
            [root_widget_ptr]() {
                return 0.5*root_widget_ptr->height.Get();
            };

    right_view0->z = mm(1);

    right_view0->color = glm::vec3{15,15,15};

    right_view0->clip = true;


    auto right_view1 =
            MakeWidget<Rectangle>(
                c.scene->GetRootWidget(),
                "right_view1");

    right_view1->x =
            [root_widget_ptr]() {
                return 0.5*root_widget_ptr->width.Get();
            };

    right_view1->y =
            [root_widget_ptr]() {
                return 0.5*root_widget_ptr->height.Get();
            };

    right_view1->width =
            [root_widget_ptr]() {
                return 0.5*root_widget_ptr->width.Get();
            };

    right_view1->height =
            [root_widget_ptr]() {
                return 0.5*root_widget_ptr->height.Get();
            };

    right_view1->color = glm::vec3{25,25,25};

    right_view1->clip = true;


    // Add some long stripes that exceed the boundaries of
    // their parents to show clipping
    auto yellow_stripes = CreateStripes(left_view,mm(300),glm::vec3(255,255,0));
    yellow_stripes->x = 0.5*(left_view->width.Get() - yellow_stripes->width.Get());
    yellow_stripes->y = 0.5*(left_view->height.Get() - yellow_stripes->height.Get());
    yellow_stripes->z = left_view->z.Get()+mm(0.1);

    auto red_stripes = CreateStripes(right_view0,mm(300),glm::vec3(255,51,51));
    red_stripes->x = 0.5*(right_view0->width.Get() - red_stripes->width.Get());
    red_stripes->y = 0.5*(right_view0->height.Get() - red_stripes->height.Get());
    red_stripes->z = right_view0->z.Get()+mm(0.1);

    auto blue_stripes = CreateStripes(right_view1,mm(300),glm::vec3(51,51,255));
    blue_stripes->x = 0.5*(right_view1->width.Get() - blue_stripes->width.Get());
    blue_stripes->y = 0.5*(right_view1->height.Get() - blue_stripes->height.Get());
    blue_stripes->z = right_view1->z.Get()+mm(0.1);

    std::vector<Widget*> list_stripes;
    list_stripes.push_back(yellow_stripes.get());
    list_stripes.push_back(red_stripes.get());
    list_stripes.push_back(blue_stripes.get());

    float angle_rads = 0.0f;

    // Animate the stripes
    auto anim_timer =
            ks::MakeObject<ks::CallbackTimer>(
                c.scene->GetEventLoop(),
                Milliseconds(33),
                [&list_stripes,&angle_rads]()
                {
                    angle_rads += (6.28f/360.0f)*0.5;

                    for(auto widget : list_stripes)
                    {
                        widget->rotation = angle_rads;
                    }
                });

    anim_timer->Start();


    // Run!
    c.app->Run();


    return 0;
}
