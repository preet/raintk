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

#include <raintk/test/RainTkTestDrawSystem.hpp>
#include <raintk/RainTkMainDrawStage.hpp>

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;

    auto draw_system = c.scene->GetDrawSystem();
    auto transform_system = c.scene->GetTransformSystem();

    SetupDrawSystem(draw_system);

    // Disable clipping from the DrawSystem so
    // we can test custom clips
    draw_system->SetClippingEnabled(false);

    // Create a clip region centered in the window
    // (clip regions are BL,TR, not x,y,w,h)
    std::vector<BoundingBox> list_clip_regions;
    list_clip_regions.emplace_back(BoundingBox{0,0,px(600),px(360)});
    list_clip_regions.emplace_back(BoundingBox{px(100),px(50),px(500),px(300)});

    // Sync clips
    draw_system->m_list_clip_regions = list_clip_regions;

    // Set clip index
    opq_draw_key.SetClip(1);

    std::vector<glm::u8vec4> list_yellow_colors;
    list_yellow_colors.push_back(glm::u8vec4(255,255,0,255));
    list_yellow_colors.push_back(glm::u8vec4(255,175,0,255));

    uint triangle_count=0;

    for(uint i=0; i < 10; i++)
    {
        for(uint j=0; j < 20; j++)
        {
            glm::u8vec4 const &color = (triangle_count%2==0) ?
                        list_yellow_colors[0] :
                        list_yellow_colors[1];

            float k = 55;

            UPtrBuffer vx_buffer_triangle =
                    CreateTriangle(glm::vec4(k*j, k*i, 0, 1),
                                   glm::vec4(k*j, k*i + k, 0, 1),
                                   glm::vec4(k*j + k, k*i + k, 0, 1),
                                   color);

            Id triangle_ent_id = c.scene->CreateEntity();

            draw_system->GetDrawDataComponentList()->Create(
                        triangle_ent_id,
                        DrawData{
                            opq_draw_key,
                            std::move(vx_buffer_triangle),
                            true
                        });

            TransformData xf_data;
            xf_data.world_xf = glm::mat4(1.0);

            transform_system->GetTransformDataComponentList()->Create(
                        triangle_ent_id,
                        xf_data);

            triangle_count++;
        }
    }

    c.app->Run();

    return 0;
}

