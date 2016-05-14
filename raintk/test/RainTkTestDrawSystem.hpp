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

#include <raintk/RainTkDrawSystem.hpp>
#include <raintk/RainTkTransformSystem.hpp>
#include <raintk/test/RainTkTestContext.hpp>
#include <raintk/shaders/RainTkColorAttr.glsl.hpp>

using namespace raintk;
using AttrType = ks::gl::VertexBuffer::Attribute::Type;

namespace
{
    // ============================================================= //

    struct SimpleVertex
    {
        glm::vec4 a_v4_position;
        glm::u8vec4 a_v4_color;
    };

    // ============================================================= //

    UPtrBuffer CreateTriangle(glm::vec4 a,
                              glm::vec4 b,
                              glm::vec4 c,
                              glm::u8vec4 color=glm::u8vec4(255,255,0,255))
    {
        UPtrBuffer vx_buffer_triangle;
        vx_buffer_triangle = make_unique<std::vector<u8>>();

        // The first and last vertices are doubled to introduce
        // degenerate triangles in the triangle strip

        ks::gl::Buffer::PushElement<SimpleVertex>(
                    *vx_buffer_triangle,
                    SimpleVertex{
                        a,
                        color});

        ks::gl::Buffer::PushElement<SimpleVertex>(
                    *vx_buffer_triangle,
                    SimpleVertex{
                        a,
                        color});

        ks::gl::Buffer::PushElement<SimpleVertex>(
                    *vx_buffer_triangle,
                    SimpleVertex{
                        b,
                        color});

        ks::gl::Buffer::PushElement<SimpleVertex>(
                    *vx_buffer_triangle,
                    SimpleVertex{
                        c,
                        color});

        ks::gl::Buffer::PushElement<SimpleVertex>(
                    *vx_buffer_triangle,
                    SimpleVertex{
                        c,
                        color});

        return vx_buffer_triangle;
    }

    // ============================================================= //

    UPtrBuffer CreateRectangle(glm::vec4 tl,
                               glm::vec4 br,
                               glm::u8vec4 color=glm::u8vec4(255,255,0,255))
    {
        UPtrBuffer vx_buffer_rect =
                make_unique<std::vector<u8>>();

        glm::vec4 bl(tl.x,br.y,0.0f,1.0f);
        glm::vec4 tr(br.x,tl.y,0.0f,1.0f);

        // The first and last vertices are doubled to introduce
        // degenerate triangles in the triangle strip

        ks::gl::Buffer::PushElement<SimpleVertex>(
                    *vx_buffer_rect,
                    SimpleVertex{
                        tl,color
                    });

        ks::gl::Buffer::PushElement<SimpleVertex>(
                    *vx_buffer_rect,
                    SimpleVertex{
                        tl,color
                    });

        ks::gl::Buffer::PushElement<SimpleVertex>(
                    *vx_buffer_rect,
                    SimpleVertex{
                        bl,color
                    });

        ks::gl::Buffer::PushElement<SimpleVertex>(
                    *vx_buffer_rect,
                    SimpleVertex{
                        tr,color
                    });

        ks::gl::Buffer::PushElement<SimpleVertex>(
                    *vx_buffer_rect,
                    SimpleVertex{
                        br,color
                    });

        ks::gl::Buffer::PushElement<SimpleVertex>(
                    *vx_buffer_rect,
                    SimpleVertex{
                        br,color
                    });

        return vx_buffer_rect;
    }

    // ============================================================= //

    UPtrBuffer CreateLine(glm::vec4 v4_a,
                          glm::vec4 v4_b,
                          glm::u8vec4 color=glm::u8vec4(255,255,0,255))
    {
        glm::vec3 a(v4_a);
        glm::vec3 b(v4_b);

        glm::vec3 ab_dirn = b-a;
        glm::vec3 ab_norm(ab_dirn.y*-1.0f,ab_dirn.x,0.0); // 2d perp
        ab_norm = glm::normalize(ab_norm)*0.33f;

        // ('left' and 'right' are arbitrary here)
        glm::vec4 tl(a+ab_norm,1.0);
        glm::vec4 bl(b+ab_norm,1.0);
        glm::vec4 tr(a-ab_norm,1.0);
        glm::vec4 br(b-ab_norm,1.0);

        UPtrBuffer vx_buffer_line;
        vx_buffer_line = make_unique<std::vector<u8>>();


        // The first and last vertices are doubled to introduce
        // degenerate triangles in the triangle strip
        ks::gl::Buffer::PushElement<SimpleVertex>(
                    *vx_buffer_line,
                    SimpleVertex{
                        tl,color});

        ks::gl::Buffer::PushElement<SimpleVertex>(
                    *vx_buffer_line,
                    SimpleVertex{
                        tl,color});

        ks::gl::Buffer::PushElement<SimpleVertex>(
                    *vx_buffer_line,
                    SimpleVertex{
                        bl,color});

        ks::gl::Buffer::PushElement<SimpleVertex>(
                    *vx_buffer_line,
                    SimpleVertex{
                        tr,color});

        ks::gl::Buffer::PushElement<SimpleVertex>(
                    *vx_buffer_line,
                    SimpleVertex{
                        br,color});

        ks::gl::Buffer::PushElement<SimpleVertex>(
                    *vx_buffer_line,
                    SimpleVertex{
                        br,color});

        return vx_buffer_line;
    }

    // ============================================================= //
    // ============================================================= //

    shared_ptr<GeometryLayout> geometry_layout(
        new GeometryLayout{
        ks::gl::VertexLayout
        {
            {
                "a_v4_position",
                AttrType::Float,
                4,
                false
            },
            {
                "a_v4_color",
                AttrType::UByte,
                4,
                true
            }
        },
        sizeof(SimpleVertex),
        50000 // buffer size: 50000 bytes
    });

    Id color_attr_shader_id;

    Id opq_depth_config_id;
    Id opq_blend_config_id;
    Id geometry_layout_id;

    DrawKey opq_draw_key;

    // ============================================================= //
    // ============================================================= //

    Id xpr_depth_config_id;
    Id xpr_blend_config_id;

    DrawKey xpr_draw_key;

    void SetupDrawSystem(DrawSystem* draw_system)
    {
        color_attr_shader_id =
                draw_system->RegisterShader(
                    "color_attr",
                    color_attr_vert_glsl,
                    color_attr_frag_glsl);

        geometry_layout_id =
                draw_system->RegisterGeometryLayout(
                    geometry_layout);

        // Opaque
        opq_depth_config_id =
                draw_system->RegisterDepthConfig(
                    [](ks::gl::StateSet* state_set) {
                        state_set->SetDepthMask(GL_TRUE);
                        state_set->SetDepthTest(GL_TRUE);
                    });

        opq_blend_config_id =
                draw_system->RegisterBlendConfig(
                    [](ks::gl::StateSet* state_set) {
                        state_set->SetBlend(GL_FALSE);
                    });

        opq_draw_key.SetShader(color_attr_shader_id);
        opq_draw_key.SetDepthConfig(opq_depth_config_id);
        opq_draw_key.SetBlendConfig(opq_blend_config_id);
        opq_draw_key.SetPrimitive(ks::gl::Primitive::TriangleStrip);
        opq_draw_key.SetGeometryLayout(geometry_layout_id);
        opq_draw_key.SetTransparency(false);

        // Transparent
        xpr_depth_config_id =
                draw_system->RegisterDepthConfig(
                    [](ks::gl::StateSet* state_set) {
                        state_set->SetDepthMask(GL_FALSE);
                        state_set->SetDepthTest(GL_TRUE);
                    });

        xpr_blend_config_id =
                draw_system->RegisterBlendConfig(
                    [](ks::gl::StateSet* state_set) {
                        state_set->SetBlend(GL_TRUE);
                        state_set->SetBlendFunction(
                            GL_SRC_ALPHA,
                            GL_ONE_MINUS_SRC_ALPHA,
                            GL_SRC_ALPHA,
                            GL_ONE_MINUS_SRC_ALPHA);
                    });

        xpr_draw_key.SetShader(color_attr_shader_id);
        xpr_draw_key.SetDepthConfig(xpr_depth_config_id);
        xpr_draw_key.SetBlendConfig(xpr_blend_config_id);
        xpr_draw_key.SetPrimitive(ks::gl::Primitive::TriangleStrip);
        xpr_draw_key.SetGeometryLayout(geometry_layout_id);
        xpr_draw_key.SetTransparency(true);
    }
}
