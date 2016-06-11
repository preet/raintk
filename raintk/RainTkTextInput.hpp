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

#ifndef RAINTK_TEXT_INPUT_HPP
#define RAINTK_TEXT_INPUT_HPP

#include <raintk/RainTkInputArea.hpp>
#include <raintk/RainTkText.hpp>
#include <raintk/RainTkRectangle.hpp>

namespace raintk
{
    // =========================================================== //

    class TextInputMultipleLines : public ks::Exception
    {
    public:
        TextInputMultipleLines();
        ~TextInputMultipleLines() = default;
    };

    // =========================================================== //

    class InvalidCursorPosition : public ks::Exception
    {
    public:
        InvalidCursorPosition();
        ~InvalidCursorPosition() = default;
    };

    // =========================================================== //

    class TextInputScrollAnim;
    class TextInputCursorAnim;

    class TextInput : public InputArea
    {
    public:
        using base_type = raintk::InputArea;

        TextInput(ks::Object::Key const &key,
                        Scene* scene,
                        shared_ptr<Widget> parent);

        void Init(ks::Object::Key const &,
                  shared_ptr<TextInput> const &);

        ~TextInput();

        shared_ptr<Text> const &GetInputText() const;

        // Properties

        // * The utf16text index the cursor is positioned before
        // * The last cursor position is after the final utf16 index
        Property<uint> cursor_position;

        Property<float> cursor_width;

        Property<glm::u8vec4> cursor_color;

    private:
        void onInputFocusChanged() override;

        // Keyboard Input
        void handleKeyboardInput(ks::gui::KeyEvent const &key_event) override;
        void handleUTF8Input(std::string const &utf8text) override;

        // Mouse/Touch Input
        Response handleInput(Point const &new_point,bool inside) override;
        void cancelInput() override;

        // Cursor and text manipulation
        uint peekCursorNext();
        uint peekCursorPrev();
        void moveCursorLeft();
        void moveCursorRight();
        void moveCursorNext();
        void moveCursorPrev();
        void deletePrev();
        void deleteNext();

        void onGlyphDataChanged();
        void onCursorWidthChanged();
        void onCursorColorChanged();
        void onCursorPositionChanged();
        void setCursorFromInputPoint(float input_x, float input_y);
        void setContentX(float new_x);

        struct GlyphDims
        {
            bool valid;
            bool rtl;
            float x0;
            float x1;
        };

        uint k_max_int = std::numeric_limits<uint>::max();

        bool m_inside_drag{false};
        float m_input_x{0.0f};
        float m_input_y{0.0f};

        std::u16string const * m_utf16_text{nullptr};
        ks::text::Line const * m_text_line{nullptr};

        shared_ptr<Widget> m_content_parent;
        shared_ptr<Text> m_input_text;
        shared_ptr<Rectangle> m_cursor;

        // Each index of @m_list_glyph_dims corresponds to an
        // index in utf16_text. The 'valid' field of GlyphDims
        // indicates whether or not a glyph actually exists (ie
        // it might be an index in the middle of a codepoint)
        std::vector<GlyphDims> m_list_glyph_dims;

        // The scrolling animation for dragging outside the
        // InputText bounds
        shared_ptr<TextInputScrollAnim> m_scroll_anim;
        shared_ptr<TextInputCursorAnim> m_cursor_blink_anim;
    };

    // =========================================================== //
}

#endif // RAINTK_TEXT_INPUT_HPP
