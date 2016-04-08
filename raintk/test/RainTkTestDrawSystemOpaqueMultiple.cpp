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

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;

    auto draw_system = c.scene->GetDrawSystem();
    auto transform_system = c.scene->GetTransformSystem();
    geometry_layout->buffer_size_hint_bytes = 500;

    SetupDrawSystem(draw_system);

    // VERIFY: There should be two draw calls, one for
    // each buffer -- buffer size is 500 bytes and 6
    // triangles exceeds that (600 bytes)
    for(uint i=0; i < 6; i++)
    {
        UPtrBuffer vx_buffer_triangle =
                CreateTriangle(glm::vec4(20*i,0,0,1),
                               glm::vec4(20*i,20,0,1),
                               glm::vec4(20*(i+1),20,0,1));

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
    }



    c.app->Run();

    return 0;
}
