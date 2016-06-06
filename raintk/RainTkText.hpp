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
            glm::vec2 a_v2_tex0; // 8

            // [1] contains a value indicating whether or not
            //     the highlight color should be used (0 or 1)
            // [2] contains the index into a uniform array containing
            //     transform and color information
            glm::u16vec2 a_v2_highlight_index; // 4
        };
    }

    // ============================================================= //

    class Text : public DrawableWidget
    {
    public:
        static uint const k_max_line_width{
            std::numeric_limits<uint>::max()/2};

        enum class Alignment : u8
        {
            Auto=0,
            Left=1,
            Right=2,
            Center=3
        };

        enum class HeightCalc
        {
            // Calculate the height of this Text based on
            // glyph position and dimensions. The max and
            // min height will be at the very edge of the
            // glyph bounds. This is the default
            GlyphBounds,

            // Calculate the height of this Text based on
            // font metrics. The max and min height may not
            // fall on the edge of the glyphs and usually
            // leaves additional space above and below
            FontBounds
        };

        using base_type = DrawableWidget;

        Text(ks::Object::Key const &key,
             Scene* scene,
             shared_ptr<Widget> parent);

        void Init(ks::Object::Key const &,
                  shared_ptr<Text> const &);

        ~Text();

        ks::text::Hint& GetTextHint();
        std::u16string const &GetUTF16Text() const;
        std::vector<ks::text::Line> const * GetGlyphData() const;

        // Keeps glyph data around even after it has been
        // used to create drawables.
        void SetKeepGlyphData(bool enabled);

        // Sets the list of characters to render with highlight_color
        void SetHighlightedText(std::vector<uint> const &utf16_indices);


        // Properties
        Property<glm::u8vec4> color {
            glm::u8vec4{51,51,51,255}
        };

        Property<glm::u8vec4> highlight_color {
            glm::u8vec4{50,150,250,255}
        };

        Property<std::string> text {
            std::string("")
        };

        Property<std::string> font {
            std::string("")
        };

        Property<float> size {
            mm(5.0f)
        };

        Property<float> line_width {
            static_cast<float>(k_max_line_width)
        };

        Property<Alignment> alignment {
            Alignment::Auto
        };

        Property<HeightCalc> height_calc{
            HeightCalc::GlyphBounds
        };


        // Signals
        ks::Signal<> signal_glyph_data_changed;

    private:
        void onColorChanged();
        void onHighlightColorChanged();
        void onTextChanged();
        void onFontChanged();
        void onSizeChanged();
        void onLineWidthChanged();
        void onAlignmentChanged();
        void onHeightCalcChanged();
        void onVisibilityChanged() override;
        void onClipIdUpdated() override;
        void onTransformUpdated() override;
        void onAccOpacityUpdated() override;
        void update() override;
        void createDrawables() override;
        void destroyDrawables() override;
        void updateDrawables() override;
        void releaseBatches();
        void acquireBatches();
        void updateColorUniforms();
        void updateTransformUniforms();
        void updateHighlight();

        void genGlyphVertexBuffers(
                std::vector<ks::text::Line> const &list_lines,
                std::vector<text_detail::GlyphBatch*> const &list_glyph_batches,
                std::vector<UPtrBuffer>& list_glyph_vx_buffs);

        void findNonZeroGlyph(
                std::vector<ks::text::Line> const &list_lines);

        static void setupTypeInit(Scene* scene);

        DrawDataComponentList* const m_cmlist_draw_data;

        Id m_cid_color;
        Id m_cid_highlight_color;
        Id m_cid_text;
        Id m_cid_font;
        Id m_cid_size;
        Id m_cid_line_width;
        Id m_cid_alignment;
        Id m_cid_height_calc;

        bool m_upd_recreate{false};
        bool m_upd_xf{false};
        bool m_upd_color{false};
        bool m_upd_highlight{false};

        std::u16string m_u16_text;
        ks::text::Hint m_text_hint;
        std::vector<text_detail::GlyphBatch> m_list_glyph_batches;
        std::vector<std::pair<std::vector<u8>*,uint>> m_lkup_utf16_vertex;
        std::vector<uint> m_utf16_highlight;

        // Vertices for the bounds of a single Non-zero glyph
        // if it exists (width and height are 0 if they are
        // invalid), used to update glyph res uniform array
        text_detail::Vertex m_nz_glyph_tl;
        text_detail::Vertex m_nz_glyph_br;

        std::unique_ptr<std::vector<ks::text::Line>> m_list_lines;

        bool m_keep_glyph_data;
    };

    // ============================================================= //
}

#endif // RAINTK_TEXT_HPP
