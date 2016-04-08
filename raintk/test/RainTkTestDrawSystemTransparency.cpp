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
#include <raintk/RainTkLog.hpp>

#include <glm/gtx/transform.hpp>

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;

    auto draw_system = c.scene->GetDrawSystem();
    auto transform_system = c.scene->GetTransformSystem();

    SetupDrawSystem(draw_system);

    // colors
    std::vector<glm::u8vec3> list_colors {
        glm::u8vec3{255,0,0},
        glm::u8vec3{255,128,0},
        glm::u8vec3{255,255,0},
        glm::u8vec3{0,255,0},
        glm::u8vec3{0,0,255},
        glm::u8vec3{128,0,255},
        glm::u8vec3{255,0,0},
        glm::u8vec3{255,128,0},
        glm::u8vec3{255,255,0},
        glm::u8vec3{0,255,0},
        glm::u8vec3{0,0,255},
        glm::u8vec3{128,0,255}
    };

     // Multiple overlapping
    {
        for(uint i=0; i < list_colors.size(); i++)
        {
            float z = 20-i;
            float dy = (i%2==0) ? 2.0f : -2.0f;
            glm::vec4 tl(5+(11*i),7+dy,z,1);
            glm::vec4 br(tl.x+15,tl.y+15,z,1);

            tl = 4.0f*tl; tl.w = 1;
            br = 4.0f*br; br.w = 1;

            UPtrBuffer vx_buffer_rect =
                    CreateRectangle(
                        tl,br,glm::vec4(list_colors[i],220));

            auto rect_ent_id = c.scene->CreateEntity();
            draw_system->GetDrawDataComponentList()->Create(
                        rect_ent_id,
                        DrawData{
                            xpr_draw_key,
                            std::move(vx_buffer_rect),
                            true
                        });

            TransformData xf_data;
            xf_data.bbox = BoundingBox{tl.x,tl.y,br.x,br.y};
            xf_data.world_xf = glm::translate(glm::vec3(0,0,z));

            transform_system->GetTransformDataComponentList()->Create(
                        rect_ent_id,
                        xf_data);
        }
    }

    c.app->Run();

    return 0;
}
