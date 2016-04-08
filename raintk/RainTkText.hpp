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

#ifndef RAINTK_TEXT_HPP
#define RAINTK_TEXT_HPP

#include <raintk/RainTkDrawableWidget.hpp>
#include <ks/text/KsTextDataTypes.hpp>

namespace ks
{
    namespace draw
    {
        struct UniformSet;
    }
}

namespace raintk
{
    // ============================================================= //

    namespace text_detail
    {
        struct GlyphBatchAvail
        {
            shared_ptr<ks::draw::UniformSet> uniform_set;
            Id uniform_set_id;
            std::vector<uint> list_avail;
        };

        struct GlyphBatchIndex
        {
            shared_ptr<ks::draw::UniformSet> uniform_set;
            Id uniform_set_id;
            uint index;
        };

        struct GlyphBatch
        {
            Id entity_id;
            shared_ptr<ks::draw::UniformSet> uniform_set;
            uint index;

            uint atlas_index;
            Id uniform_set_id;
            Id texture_set_id;
        };

        struct Vertex
        {
            glm::vec2 a_v2_position; // 8

            // Components [0] and [1] contain the x,y texture
            // coordinates. [2] contains an index into a uniform
            // array containing transform and color information.
            glm::vec3 a_v3_tex0_index; // 12
        };
    }

    // ============================================================= //

    class Text : public DrawableWidget
    {
        uint k_max_line_width{std::numeric_limits<uint>::max()/2};

    public:
        enum class Alignment : u8
        {
            Auto=0,
            Left=1,
            Right=2,
            Center=3
        };

        using base_type = DrawableWidget;

        Text(ks::Object::Key const &key,
             shared_ptr<Widget> parent,
             std::string name);

        void Init(ks::Object::Key const &,
                  shared_ptr<Text> const &);

        ~Text();

        ks::text::Hint& GetTextHint();

        // Properties
        Property<glm::u8vec3> color {
            name+".color",glm::u8vec3{51,51,51}
        };

        Property<std::string> text {
            name+".text",std::string("")
        };

        Property<std::string> font {
            name+".font",std::string("")
        };

        Property<float> size {
            name+".size",mm(5.0f)
        };

        Property<float> line_width {
            name+".line_width",
            static_cast<float>(k_max_line_width)
        };

        Property<Alignment> alignment {
            name+".alignment",Alignment::Auto
        };

    private:
        void onOpacityChanged() override;
        void onVisibilityChanged() override;
        void onClipIdUpdated() override;
        void onTransformUpdated() override;
        void onColorChanged();
        void onTextChanged();
        void onFontChanged();
        void onSizeChanged();
        void onLineWidthChanged();
        void onAlignmentChanged();
        void update() override;
        void createDrawables() override;
        void destroyDrawables() override;
        void updateDrawables() override;
        void releaseBatches();
        void acquireBatches();
        void updateColorUniforms();
        void updateTransformUniforms();

        void genGlyphVertexBuffers(
                std::vector<ks::text::Line> const &list_lines,
                std::vector<text_detail::GlyphBatch*> const &list_glyph_batches,
                std::vector<UPtrBuffer>& list_glyph_vx_buffs);

        void findNonZeroGlyph(
                std::vector<ks::text::Line> const &list_lines);

        static void setupTypeInit(Scene* scene);

        DrawDataComponentList* const m_cmlist_draw_data;

        Id m_cid_color;
        Id m_cid_text;
        Id m_cid_font;
        Id m_cid_size;
        Id m_cid_line_width;
        Id m_cid_alignment;

        bool m_upd_recreate{false};
        bool m_upd_xf{false};
        bool m_upd_color{false};

        std::u16string m_u16_text;
        ks::text::Hint m_text_hint;
        std::vector<text_detail::GlyphBatch> m_list_glyph_batches;

        // Vertices for the bounds of a single Non-zero glyph
        // if it exists (width and height are 0 if they are
        // invalid), used to update glyph res uniform array
        text_detail::Vertex m_nz_glyph_tl;
        text_detail::Vertex m_nz_glyph_br;

        std::unique_ptr<std::vector<ks::text::Line>> m_list_lines;
    };

    // ============================================================= //
}

#endif // RAINTK_TEXT_HPP
