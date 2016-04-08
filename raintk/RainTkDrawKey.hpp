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

#ifndef RAINTK_DRAW_KEY_HPP
#define RAINTK_DRAW_KEY_HPP

#include <ks/gl/KsGLVertexBuffer.hpp>
#include <raintk/RainTkGlobal.hpp>

namespace raintk
{
    class DrawKey final
    {
    public:
        // [bits from LSBit to MSBit]
        // [3]: Primitive
        // [6]: UniformSet
        // [9]: TextureSet
        // [4]: StencilConfig
        // [6]: BlendConfig
        // [4]: DepthConfig
        // [5]: Shader
        // [10]: Clip

        // number of bits
        static const u8 k_bits_primitive        = 3;
        static const u8 k_bits_uniform_set      = 6;
        static const u8 k_bits_texture_set      = 9;
        static const u8 k_bits_stencil_config   = 4;
        static const u8 k_bits_blend_config     = 6;
        static const u8 k_bits_depth_config     = 4;
        static const u8 k_bits_shader           = 5;
        static const u8 k_bits_clip             = 8;
        static const u8 k_bits_gm_layout        = 6;
        static const u8 k_bits_transparency     = 1;

        // starting bit position
        static const u8 k_sbit_primitive        = 0;
        static const u8 k_sbit_uniform_set      = k_sbit_primitive+k_bits_primitive;
        static const u8 k_sbit_texture_set      = k_sbit_uniform_set+k_bits_uniform_set;
        static const u8 k_sbit_stencil_config   = k_sbit_texture_set+k_bits_texture_set;
        static const u8 k_sbit_blend_config     = k_sbit_stencil_config+k_bits_stencil_config;
        static const u8 k_sbit_depth_config     = k_sbit_blend_config+k_bits_blend_config;
        static const u8 k_sbit_shader           = k_sbit_depth_config+k_bits_depth_config;
        static const u8 k_sbit_clip             = k_sbit_shader+k_bits_shader;
        static const u8 k_sbit_gm_layout        = k_sbit_clip+k_bits_clip;
        static const u8 k_sbit_transparency     = k_sbit_gm_layout+k_bits_gm_layout;

        // mask
        template<u8 sbit,u8 bits>
        struct mask {
            static const u64 value;
        };

        // map int to primitive type
        static const std::vector<ks::gl::Primitive> k_list_prim_to_int;

    public:
        Id GetKey() const { return m_key; }

        bool GetTransparency() const;
        Id GetGeometryLayout() const;
        Id GetClip() const;
        Id GetShader() const;
        Id GetDepthConfig() const;
        Id GetBlendConfig() const;
        Id GetStencilConfig() const;
        Id GetTextureSet() const;
        Id GetUniformSet() const;
        ks::gl::Primitive GetPrimitive() const;

        void SetTransparency(bool transparency);
        void SetGeometryLayout(Id gm_layout);
        void SetClip(Id clip);
        void SetShader(Id shader);
        void SetDepthConfig(Id depth_config);
        void SetBlendConfig(Id blend_config);
        void SetStencilConfig(Id stencil_config);
        void SetTextureSet(Id texture_set);
        void SetUniformSet(Id uniform_set);
        void SetPrimitive(ks::gl::Primitive primitive);

        bool operator < (DrawKey const &right) const
        {
            return (this->m_key < right.m_key);
        }

        bool operator == (DrawKey const &other) const
        {
            return (this->m_key == other.m_key);
        }

    private:
        void setData(u64 mask, u8 sbit, Id data)
        {
            m_key &= (~mask);
            m_key |= ((data << sbit) & mask);
        }

        Id m_key{0};
    };

    template<u8 sbit,u8 bits>
    u64 const DrawKey::mask<sbit,bits>::value =
            ((u64(1) << bits)-1) << sbit;
}

#endif // RAINTK_DRAW_KEY_HPP
