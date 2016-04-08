/*
   Copyright (C) 2015 Preet Desai (preet.desai@gmail.com)

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

#include <raintk/RainTkDrawKey.hpp>

namespace raintk
{

    const std::vector<ks::gl::Primitive> DrawKey::k_list_prim_to_int = {
        ks::gl::Primitive::Triangles,       // 0
        ks::gl::Primitive::TriangleFan,     // 1
        ks::gl::Primitive::TriangleStrip,   // 2
        ks::gl::Primitive::Lines,           // 3
        ks::gl::Primitive::LineLoop,        // 4
        ks::gl::Primitive::LineStrip,       // 5
        ks::gl::Primitive::Points           // 6
    };

    bool DrawKey::GetTransparency() const
    {
        uint transparency = ((m_key & mask<k_sbit_transparency,k_bits_transparency>::value) >> k_sbit_transparency);
        return (transparency == 1);
    }

    Id DrawKey::GetGeometryLayout() const
    {
        return ((m_key & mask<k_sbit_gm_layout,k_bits_gm_layout>::value) >> k_sbit_gm_layout);
    }

    Id DrawKey::GetClip() const
    {
        return ((m_key & mask<k_sbit_clip,k_bits_clip>::value) >> k_sbit_clip);
    }

    Id DrawKey::GetShader() const
    {
        return ((m_key & mask<k_sbit_shader,k_bits_shader>::value) >> k_sbit_shader);
    }

    Id DrawKey::GetDepthConfig() const
    {
        return ((m_key & mask<k_sbit_depth_config,k_bits_depth_config>::value) >> k_sbit_depth_config);
    }

    Id DrawKey::GetBlendConfig() const
    {
        return ((m_key & mask<k_sbit_blend_config,k_bits_blend_config>::value) >> k_sbit_blend_config);
    }

    Id DrawKey::GetStencilConfig() const
    {
        return ((m_key & mask<k_sbit_stencil_config,k_bits_stencil_config>::value) >> k_sbit_stencil_config);
    }

    Id DrawKey::GetTextureSet() const
    {
        return ((m_key & mask<k_sbit_texture_set,k_bits_texture_set>::value) >> k_sbit_texture_set);
    }

    Id DrawKey::GetUniformSet() const
    {
        return ((m_key & mask<k_sbit_uniform_set,k_bits_uniform_set>::value) >> k_sbit_uniform_set);
    }

    ks::gl::Primitive DrawKey::GetPrimitive() const
    {
        Id primitive = ((m_key & mask<k_sbit_primitive,k_bits_primitive>::value) >> k_sbit_primitive);
        return k_list_prim_to_int[primitive];
    }

    // ============================================================= //

    void DrawKey::SetTransparency(bool transparency)
    {
        setData(mask<k_sbit_transparency,k_bits_transparency>::value,
                k_sbit_transparency,
                static_cast<uint>(transparency));
    }

    void DrawKey::SetGeometryLayout(Id gm_layout)
    {       
        setData(mask<k_sbit_gm_layout,k_bits_gm_layout>::value, k_sbit_gm_layout, gm_layout);
    }

    void DrawKey::SetClip(Id clip)
    {
        setData(mask<k_sbit_clip,k_bits_clip>::value, k_sbit_clip, clip);
    }

    void DrawKey::SetShader(Id shader)
    {
        setData(mask<k_sbit_shader,k_bits_shader>::value, k_sbit_shader, shader);
    }

    void DrawKey::SetDepthConfig(Id depth_config)
    {
        setData(mask<k_sbit_depth_config,k_bits_depth_config>::value, k_sbit_depth_config, depth_config);
    }

    void DrawKey::SetBlendConfig(Id blend_config)
    {
        setData(mask<k_sbit_blend_config,k_bits_blend_config>::value, k_sbit_blend_config, blend_config);
    }

    void DrawKey::SetStencilConfig(Id stencil_config)
    {
        setData(mask<k_sbit_stencil_config,k_bits_stencil_config>::value, k_sbit_stencil_config, stencil_config);
    }

    void DrawKey::SetTextureSet(Id texture_set)
    {
        setData(mask<k_sbit_texture_set,k_bits_texture_set>::value, k_sbit_texture_set, texture_set);
    }

    void DrawKey::SetUniformSet(Id uniform_set)
    {
        setData(mask<k_sbit_uniform_set,k_bits_uniform_set>::value, k_sbit_uniform_set, uniform_set);
    }

    void DrawKey::SetPrimitive(ks::gl::Primitive primitive)
    {
        Id primitive_int=0;

        if(primitive == ks::gl::Primitive::Triangles) {
            primitive_int = 0;
        }
        else if(primitive == ks::gl::Primitive::TriangleFan) {
            primitive_int = 1;
        }
        else if(primitive == ks::gl::Primitive::TriangleStrip) {
            primitive_int = 2;
        }
        else if(primitive == ks::gl::Primitive::Lines) {
            primitive_int = 3;
        }
        else if(primitive == ks::gl::Primitive::LineLoop) {
            primitive_int = 4;
        }
        else if(primitive == ks::gl::Primitive::LineStrip) {
            primitive_int = 5;
        }
        else if(primitive == ks::gl::Primitive::Points) {
            primitive_int = 6;
        }

        setData(mask<k_sbit_primitive,k_bits_primitive>::value, k_sbit_primitive, primitive_int);
    }
}
