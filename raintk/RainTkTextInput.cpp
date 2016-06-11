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

#include <ks/gui/KsGuiInput.hpp>
#include <ks/text/KsTextTextManager.hpp>

#include <raintk/RainTkScene.hpp>
#include <raintk/RainTkTextInput.hpp>
#include <raintk/RainTkAlignment.hpp>
#include <raintk/RainTkAnimation.hpp>
#include <raintk/RainTkAnimationSystem.hpp>
#include <raintk/RainTkTransformSystem.hpp>

namespace raintk
{
    // =========================================================== //

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

    TextInputMultipleLines::TextInputMultipleLines() :
        ks::Exception(ks::Exception::ErrorLevel::ERROR,
                      "TextInput: Input text is multiple lines ("
                      "only single lined text is allowed)")
    {}

    // =========================================================== //

    InvalidCursorPosition::InvalidCursorPosition() :
        ks::Exception(ks::Exception::ErrorLevel::ERROR,"")
    {}

    // =========================================================== //

    class TextInputScrollAnim : public Animation
    {
    public:
        using base_type = raintk::Animation;

        TextInputScrollAnim(ks::Object::Key const &key,
                            Scene* scene,
                            std::function<void(float)> callback) :
            Animation(key,scene),
            m_callback(std::move(callback))
        {}

        void Init(ks::Object::Key const &,
                  shared_ptr<TextInputScrollAnim> const &)
        {}

        ~TextInputScrollAnim()
        {}

        void start() override
        {}

        bool update(float) override
        {
            m_callback(x_velocity);
            return false;
        }

        void complete() override
        {}

        void SetVelocity(float x_vel)
        {
            x_velocity = x_vel;

            if(x_velocity == 0.0f)
            {
                this->Stop();
            }
            else
            {
                this->Start();
            }
        }

        float x_velocity{0.0f};

    private:
        std::function<void(float)> m_callback;
    };

    // =========================================================== //

    class TextInputCursorAnim : public Animation
    {
    public:
        using base_type = raintk::Animation;

        TextInputCursorAnim(ks::Object::Key const &key,
                            Scene* scene,
                            Rectangle* cursor) :
            Animation(key,scene),
            m_cursor(cursor)
        {
            opacities[0] = 1.0f;
            opacities[1] = 0.0f;
        }

        void Init(ks::Object::Key const &,
                  shared_ptr<TextInputCursorAnim> const &)
        {}

        ~TextInputCursorAnim()
        {}

        void start() override
        {
            elapsed_ms = 0.0f;
        }

        bool update(float delta_ms) override
        {
            elapsed_ms += delta_ms;
            if(elapsed_ms > 600.0f)
            {
                elapsed_ms = 0.0f;

                opacity_index = 1-opacity_index;

                if(m_cursor->opacity.Get() != opacities[opacity_index])
                {
                    m_cursor->opacity = opacities[opacity_index];
                }
            }

            return false;
        }

        void complete() override
        {}

        float opacities[2];
        uint opacity_index{0};
        float elapsed_ms{0.0f};

    private:
        Rectangle* const m_cursor;
    };


    // =========================================================== //

    TextInput::TextInput(ks::Object::Key const &key,
                         Scene* scene,
                         shared_ptr<Widget> parent) :
        InputArea(key,scene,parent)
    {
        // Set some default content dimensions
        width = mm(40);
        height = mm(10);
    }

    void TextInput::Init(ks::Object::Key const &,
                    shared_ptr<TextInput> const &this_text_input)
    {
        m_content_parent = MakeWidget<Widget>(m_scene,this_text_input);

        m_input_text = MakeWidget<Text>(m_scene,m_content_parent);
        m_input_text->SetKeepGlyphData(true);
        m_input_text->height_calc = Text::HeightCalc::FontBounds;
        m_input_text->z = 0.1f; // So we're above the cursor

        m_input_text->signal_glyph_data_changed.Connect(
                    this_text_input,
                    &TextInput::onGlyphDataChanged,
                    ks::ConnectionType::Direct);

        m_content_parent->width =
                [this](){
                    return m_input_text->width.Get();
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

        cursor_width.signal_changed.Connect(
                    this_text_input,
                    &TextInput::onCursorWidthChanged,
                    ks::ConnectionType::Direct);

        cursor_color.signal_changed.Connect(
                    this_text_input,
                    &TextInput::onCursorColorChanged,
                    ks::ConnectionType::Direct);

        m_scroll_anim =
                ks::MakeObject<TextInputScrollAnim>(
                    m_scene,
                    [this](float x_shift){
                        auto new_content_x = m_content_parent->x.Get()+x_shift;
                        setContentX(new_content_x);
                        setCursorFromInputPoint(m_input_x,m_input_y);
                    });

        m_cursor_blink_anim =
                ks::MakeObject<TextInputCursorAnim>(
                    m_scene,
                    m_cursor.get());
    }

    TextInput::~TextInput()
    {
        rtklog.Trace() << "~TextInput";
    }

    shared_ptr<Text> const & TextInput::GetInputText() const
    {
        return m_input_text;
    }

    void TextInput::onInputFocusChanged()
    {
        Widget::onInputFocusChanged();

        if(input_focus.Get())
        {
            // TODO when visibility is working
            // m_cursor->visible = true;
            m_cursor->opacity = 1.0f;
            m_cursor_blink_anim->Start();
        }
        else
        {
            // TODO when visibility is working
            // m_cursor->visible = false;
            m_cursor_blink_anim->Stop();
            m_cursor->opacity = 0.0f;
        }
        m_cursor_blink_anim->elapsed_ms = 600.0f;
    }

    void TextInput::handleKeyboardInput(ks::gui::KeyEvent const &key_event)
    {
        if(key_event.action == ks::gui::KeyEvent::Action::Press ||
           key_event.action == ks::gui::KeyEvent::Action::Repeat)
        {
            // Stop blinking
            m_cursor_blink_anim->Stop();
            m_cursor->opacity = 1.0f; // TODO FIX

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
        else if(key_event.action == ks::gui::KeyEvent::Action::Release)
        {
            m_cursor_blink_anim->Start();
        }
    }

    void TextInput::handleUTF8Input(std::string const &utf8text)
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

    InputArea::Response TextInput::handleInput(Point const &new_point,bool inside)
    {
        Response response = Response::Ignore;

        if(!m_inside_drag)
        {
            // Transition: press within area
            if(new_point.action == Point::Action::Press && inside)
            {
                if(input_focus.Get() == false)
                {
                    input_focus = true;
                }

                m_cursor_blink_anim->Stop();
                m_cursor->opacity = 1.0f;

                m_inside_drag = true;
                m_input_x = new_point.x;
                m_input_y = new_point.y;
                setCursorFromInputPoint(new_point.x,new_point.y);
                response = Response::Accept;
            }
        }
        else
        {
            m_input_x = new_point.x;
            m_input_y = new_point.y;

            // Use an animation to scroll the text. The speed
            // of the animation is based on the distance of the
            // input point x with respect to the widget edges
            if(new_point.action == Point::Action::Release)
            {
                m_inside_drag = false;
                m_scroll_anim->SetVelocity(0.0f);
                m_cursor_blink_anim->Start();
            }
            else
            {
                if(inside)
                {
                    m_scroll_anim->SetVelocity(0.0f);
                    setCursorFromInputPoint(m_input_x,m_input_y);
                }
                else
                {
                    float dist = 0.0f;

                    if(new_point.x < 0.0f)
                    {
                        dist = new_point.x;
                    }
                    else if(new_point.x > width.Get())
                    {
                        dist = new_point.x-width.Get();
                    }

                    dist *= -0.25f;
                    m_scroll_anim->SetVelocity(dist);
                }
            }

            response = Response::Accept;
        }

        return response;
    }

    void TextInput::cancelInput()
    {
        m_inside_drag = false;
        m_scroll_anim->SetVelocity(0.0f);
    }

    uint TextInput::peekCursorNext()
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

    uint TextInput::peekCursorPrev()
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

    void TextInput::moveCursorLeft()
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

    void TextInput::moveCursorRight()
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

    void TextInput::moveCursorNext()
    {
        cursor_position = peekCursorNext();
    }

    void TextInput::moveCursorPrev()
    {
        cursor_position = peekCursorPrev();
    }

    void TextInput::deletePrev()
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

    void TextInput::deleteNext()
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

    void TextInput::onGlyphDataChanged()
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

    void TextInput::onCursorWidthChanged()
    {
        m_cursor->width = cursor_width.Get();
    }

    void TextInput::onCursorColorChanged()
    {
        m_cursor->color = cursor_color.Get();
    }

    void TextInput::onCursorPositionChanged()
    {
        if(m_list_glyph_dims.empty())
        {
            m_cursor->x = 0.0f;
            return;
        }

        // Lookup the glyph
        auto new_position = cursor_position.Get();
        auto const &glyph = m_list_glyph_dims[new_position];

        // Get the x position
        float cursor_x = 0.0f;

        if(glyph.rtl)
        {
            cursor_x = glyph.x1+m_cursor->width.Get();
        }
        else
        {
            cursor_x = glyph.x0-m_cursor->width.Get();
        }

        float content_cursor_x = m_content_parent->x.Get()+cursor_x;

        // Adjust the content parent position if required
        if(content_cursor_x > width.Get())
        {
            float x_shift = (content_cursor_x - width.Get());
            setContentX(m_content_parent->x.Get() - x_shift);
        }
        else if(content_cursor_x < 0.0f)
        {
            setContentX(m_content_parent->x.Get()-content_cursor_x);
        }

        m_cursor->x = cursor_x;
    }

    void TextInput::setCursorFromInputPoint(float input_x, float input_y)
    {
        (void)input_y;

        if(m_list_glyph_dims.empty())
        {
            return;
        }

        // Convert the dimensions to local coordinates
        // within the content parent
        float const cursor_width = m_cursor->width.Get();

        auto const & content_xf_data =
                m_cmlist_xf_data->GetComponent(
                    m_content_parent->GetEntityId());

        input_x -= content_xf_data.position.x;

        float left_edge = 0.0f - content_xf_data.position.x - cursor_width;
        float right_edge = left_edge + width.Get() + cursor_width;

        // Find the closest cursor index within the widget bounds
        uint closest_index=0;
        float closest_x = 0.0f;
        float closest_dist = std::numeric_limits<float>::max();

        for(uint i=0; i < m_list_glyph_dims.size(); i++)
        {
            auto const &glyph = m_list_glyph_dims[i];

            if(glyph.valid)
            {
                float cursor_x =
                        (glyph.rtl) ?
                            glyph.x1+cursor_width :
                            glyph.x0-cursor_width;

                if((cursor_x < left_edge) ||
                   (cursor_x > right_edge))
                {
                    continue;
                }

                float cursor_dist = fabs(cursor_x-input_x);

                if(cursor_dist < closest_dist)
                {
                    closest_index = i;
                    closest_dist = cursor_dist;
                    closest_x = cursor_x;
                }
            }
        }

        cursor_position = closest_index;
    }

    void TextInput::setContentX(float new_x)
    {
        auto const delta_x = new_x-m_content_parent->x.Get();

        auto const this_width = width.Get();
        auto const content_width = m_content_parent->width.Get();

        if(content_width > this_width)
        {
            float new_x = delta_x + m_content_parent->x.Get();

            if(new_x > 0)
            {
                new_x = 0; // left edge
            }
            else if(new_x < (this_width-content_width))
            {
                new_x = this_width-content_width; // right edge
            }

            m_content_parent->x = new_x;
        }
        else
        {
            if(m_content_parent->x.Get() != 0.0f)
            {
                m_content_parent->x = 0.0f;
            }
        }
    }

    // =========================================================== //
}
