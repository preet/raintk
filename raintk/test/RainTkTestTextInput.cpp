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

#include <raintk/test/RainTkTestContext.hpp>
#include <raintk/RainTkScrollArea.hpp>
#include <raintk/RainTkRectangle.hpp>
#include <raintk/RainTkText.hpp>
#include <raintk/RainTkAlignment.hpp>

// ============================================================= //
// ============================================================= //

namespace raintk
{
    namespace
    {
        std::u16string ConvUTF8ToUTF16(std::string const &utf8text)
        {
            return ks::text::TextManager::ConvertStringUTF8ToUTF16(utf8text);
        }

        std::string ConvUTF16ToUTF8(std::u16string const &utf16text)
        {
            return ks::text::TextManager::ConvertStringUTF16ToUTF8(utf16text);
        }
    }

    // =========================================================== //

    class TextInputMultipleLines : public ks::Exception
    {
    public:
        TextInputMultipleLines() :
            ks::Exception(ks::Exception::ErrorLevel::ERROR,
                          "TextInput: Input text is multiple lines ("
                          "only single lined text is allowed)")
        {}

        ~TextInputMultipleLines() = default;
    };

    // =========================================================== //

    class InvalidCursorPosition : public ks::Exception
    {
    public:
        InvalidCursorPosition() :
            ks::Exception(ks::Exception::ErrorLevel::ERROR,"")
        {}

        ~InvalidCursorPosition() = default;
    };

    // =========================================================== //

    class TextInput : public ScrollArea
    {
    public:
        using base_type = ScrollArea;

        TextInput(ks::Object::Key const &key,
                  Scene* scene,
                  shared_ptr<Widget> parent) :
            ScrollArea(key,scene,parent)
        {
            // Set some default content dimensions
            width = mm(40);
            height = mm(10);
        }

        void Init(ks::Object::Key const &,
                  shared_ptr<TextInput> const &this_text_input)
        {
            m_input_text = MakeWidget<Text>(m_scene,m_content_parent);
            m_input_text->SetKeepGlyphData(true);
            m_input_text->z = 0.1f; // So we're above the cursor

            m_input_text->signal_glyph_data_changed.Connect(
                        this_text_input,
                        &TextInput::onGlyphDataChanged,
                        ks::ConnectionType::Direct);

            m_content_parent->width =
                    [this](){
                        return m_input_text->width.Get()+
                            2.0f*horizontal_padding.Get();
                    };

            m_content_parent->height =
                    [this](){
                        return height.Get();
                    };

            Align::BindVCenterToAnchorVCenter(
                        m_input_text.get(),
                        m_content_parent.get());


            m_cursor = MakeWidget<Rectangle>(m_scene,m_content_parent);
            m_cursor->height =
                    [this](){
                        return m_input_text->size.Get();
                    };

            m_cursor->width = 2;

            Align::BindVCenterToAnchorVCenter(
                        m_cursor.get(),
                        m_content_parent.get());



            cursor_position.signal_changed.Connect(
                        this_text_input,
                        &TextInput::onCursorPositionChanged,
                        ks::ConnectionType::Direct);
        }

        ~TextInput()
        {}


        shared_ptr<Text> const & GetInputText() const
        {
            return m_input_text;
        }


        // Properties
        Property<float> horizontal_padding{
            mm(2)
        };

        Property<float> vertical_padding{
            mm(2)
        };

        // The glyph the cursor is positioned before. The last
        // cursor position is after the final glyph.
        Property<uint> cursor_position{
            0
        };


    private:
        void handleKeyboardInput(ks::gui::KeyEvent const &key_event) override
        {
            if(key_event.action == ks::gui::KeyEvent::Action::Press ||
               key_event.action == ks::gui::KeyEvent::Action::Repeat)
            {
                if(m_input_text->text.Get().size() > 0)
                {
                    if(key_event.key == ks::gui::KeyEvent::Key::KEY_RIGHT)
                    {
                        moveCursorRight();
                    }
                    else if(key_event.key == ks::gui::KeyEvent::Key::KEY_LEFT)
                    {
                        moveCursorLeft();
                    }
                    else if(key_event.key == ks::gui::KeyEvent::Key::KEY_DELETE)
                    {
                        deleteNext();
                    }
                    else if(key_event.key == ks::gui::KeyEvent::Key::KEY_BACKSPACE)
                    {
                        deletePrev();
                    }
                    else if(key_event.key == ks::gui::KeyEvent::Key::KEY_HOME)
                    {
                        cursor_position = 0;
                    }
                    else if(key_event.key == ks::gui::KeyEvent::Key::KEY_END)
                    {
                        cursor_position = m_utf16_text->size();
                    }
                }
            }
        }

        void handleUTF8Input(std::string const &utf8text) override
        {
            // We ignore line breaking characters (0x0A to 0x0D
            // which includes CR, LF, FF, Vertical Tab) since
            // TextInput represents a single line input field
            for(auto c : utf8text)
            {
                // There should only be a single character string for
                // line breaks, but discard the whole thing just in case
                if(!(c < 0x0A || c > 0x0D))
                {
                    return;
                }
            }

            // Insert text at current cursor position
            std::u16string utf16text;

            if(m_utf16_text)
            {
                utf16text = *m_utf16_text;
            }

            auto insert_pos = cursor_position.Get();
            utf16text.insert(insert_pos,ConvUTF8ToUTF16(utf8text));
            m_input_text->text = ConvUTF16ToUTF8(utf16text);
            cursor_position = insert_pos+1;
        }

        uint peekCursorNext()
        {
            auto index = cursor_position.Get();
            while(index < m_utf16_text->size())
            {
                index++;
                if(m_list_glyph_dims[index].valid)
                {
                    break;
                }
            }

            return index;
        }

        uint peekCursorPrev()
        {
            auto index = cursor_position.Get();
            while(index > 0)
            {
                index--;
                if(m_list_glyph_dims[index].valid)
                {
                    break;
                }
            }

            return index;
        }

        void moveCursorLeft()
        {
            auto index = cursor_position.Get();
            if(m_list_glyph_dims[index].rtl)
            {
                moveCursorNext();
            }
            else
            {
                moveCursorPrev();
            }
        }

        void moveCursorRight()
        {
            auto index = cursor_position.Get();
            if(m_list_glyph_dims[index].rtl)
            {
                moveCursorPrev();
            }
            else
            {
                moveCursorNext();
            }
        }

        void moveCursorNext()
        {
            cursor_position = peekCursorNext();
        }

        void moveCursorPrev()
        {
            cursor_position = peekCursorPrev();
        }

        void deletePrev()
        {
            auto e_index = cursor_position.Get();
            moveCursorPrev();
            auto s_index = cursor_position.Get();

            if(e_index != s_index)
            {
                auto utf16_text = *m_utf16_text;

                auto s_it = std::next(utf16_text.begin(),s_index);
                auto e_it = std::next(utf16_text.begin(),e_index);

                utf16_text.erase(s_it,e_it);
                m_input_text->text = ConvUTF16ToUTF8(utf16_text);
            }
        }

        void deleteNext()
        {
            auto s_index = cursor_position.Get();
            auto e_index = peekCursorNext();

            if(e_index != s_index)
            {
                auto utf16_text = *m_utf16_text;

                auto s_it = std::next(utf16_text.begin(),s_index);
                auto e_it = std::next(utf16_text.begin(),e_index);

                utf16_text.erase(s_it,e_it);
                m_input_text->text = ConvUTF16ToUTF8(utf16_text);
            }
        }

        void onGlyphDataChanged()
        {
            m_list_glyph_dims.clear();

            if(m_input_text->text.Get().empty())
            {
                m_utf16_text = nullptr;
                m_text_line = nullptr;
            }
            else
            {
                if(m_input_text->GetGlyphData()->size() > 1)
                {
                    throw TextInputMultipleLines();
                }

                m_utf16_text = &(m_input_text->GetUTF16Text());
                m_text_line = &(m_input_text->GetGlyphData()->at(0));


                // Create the visual horizontal glyph dimensions
                // Visual is always LTR
                float const k_glyph =
                        m_input_text->size.Get()/
                        m_scene->GetTextGlyphSizePx();

                m_list_glyph_dims.resize(
                            m_utf16_text->size(),
                            GlyphDims{false,false,0,0});

                // Glyphs
                for(auto const &glyph : m_text_line->list_glyphs)
                {
                    m_list_glyph_dims[glyph.cluster] =
                            GlyphDims{
                                true,
                                glyph.rtl,
                                (glyph.x0-m_text_line->x_min)*k_glyph,
                                (glyph.x1-m_text_line->x_min)*k_glyph
                            };
                }

                m_list_glyph_dims.push_back(m_list_glyph_dims.back());
                auto& sentinel = m_list_glyph_dims.back();

                if(sentinel.rtl)
                {
                    sentinel.x1 = sentinel.x0;
                }
                else
                {
                    sentinel.x0 = sentinel.x1;
                }
            }

            // Reposition cursor
            onCursorPositionChanged();
        }

        void onCursorPositionChanged()
        {
            if(m_list_glyph_dims.empty())
            {
                m_cursor->x = 0.0f;
                return;
            }

            // Lookup the glyph
            auto new_position = cursor_position.Get();

            auto const &glyph = m_list_glyph_dims[new_position];

            if(glyph.rtl)
            {
                m_cursor->x = glyph.x1;
            }
            else
            {
                m_cursor->x = glyph.x0;
            }
        }

        std::u16string const * m_utf16_text;
        ks::text::Line const * m_text_line;

        struct GlyphDims
        {
            bool valid;
            bool rtl;
            float x0;
            float x1;
        };

        uint k_max_int = std::numeric_limits<uint>::max();


        std::vector<GlyphDims> m_list_glyph_dims;
        shared_ptr<Text> m_input_text;
        shared_ptr<Rectangle> m_cursor;
    };
}

// ============================================================= //
// ============================================================= //

using namespace raintk;

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c;

    auto scene = c.scene.get();
    auto root = c.scene->GetRootWidget();

    auto text_input = MakeWidget<TextInput>(scene,root);
    text_input->input_focus = true;
    text_input->z = 1.0f;
    text_input->GetInputText()->font = "FiraSansMinimal.ttf";
    text_input->GetInputText()->color = glm::u8vec4{250,250,250,255};
    text_input->GetInputText()->size = mm(5);
    text_input->GetInputText()->text = "HelloWorld";

    auto rect = MakeWidget<Rectangle>(scene,root);
    rect->width = [&](){ return text_input->width.Get(); };
    rect->height = [&](){ return text_input->height.Get(); };
    rect->z = 0.0f;
    rect->color = glm::u8vec4{0,0,0,255};

    // Run!
    c.app->Run();

    return 0;
}
