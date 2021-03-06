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

#ifndef RAINTK_SCROLL_BAR_HPP
#define RAINTK_SCROLL_BAR_HPP

#include <raintk/RainTkRectangle.hpp>
#include <raintk/RainTkScrollArea.hpp>

namespace raintk
{
    // Currently non-interactive, TODO allow scroll bars
    // to control content position
    class ScrollBar : public Widget
    {
    public:
        using base_type = raintk::Widget;

        enum class Direction : u8
        {
            Horizontal,
            Vertical
        };

        ScrollBar(ks::Object::Key const & key,
                  Scene* scene,
                  shared_ptr<Widget> parent,
                  Widget* view_widget,
                  Widget* content_widget) :
            Widget(key,scene,parent),
            m_view_widget(view_widget),
            m_content_widget(content_widget)
        {}

        void Init(ks::Object::Key const &,
                  shared_ptr<ScrollBar> const &this_scroll_bar)
        {
            // Setup connections
            m_cid_direction =
                    direction.signal_changed.Connect(
                        this_scroll_bar,
                        &ScrollBar::onDirectionChanged,
                        ks::ConnectionType::Direct);

            // Create a scroll track and grip
            auto track = MakeWidget<Rectangle>(m_scene,this_scroll_bar);
            track->name = "track";
            track->width = [this](){ return width.Get(); };
            track->height = [this](){ return height.Get(); };
            track->color = [this](){ return track_color.Get(); };
            track->opacity = [this](){ return track_opacity.Get(); };
            track->z = [this](){ return track_z.Get(); };

            auto grip = MakeWidget<Rectangle>(m_scene,this_scroll_bar);
            grip->name = "grip";
            grip->color = [this](){ return grip_color.Get(); };
            grip->opacity = [this](){ return grip_opacity.Get(); };
            grip->z = [this](){ return grip_z.Get(); };


            m_track = track.get();
            m_grip = grip.get();

            setupVertical();
        }

        ~ScrollBar()
        {}

        // Properties

        Property<Direction> direction{
            Direction::Vertical
        };

        Property<float> thickness{
            mm(2.0f)
        };

        Property<glm::u8vec4> track_color{
            glm::u8vec4(0,0,0,255)
        };

        Property<float> track_opacity{
            1.0f
        };

        Property<float> track_z{
            0.0f
        };

        Property<glm::u8vec4> grip_color{
            glm::u8vec4(100,100,100,255)
        };

        Property<float> grip_opacity{
            1.0f
        };

        Property<float> grip_z{
            0.1f
        };

    private:
        void onDirectionChanged()
        {
            if(direction.Get() == Direction::Horizontal)
            {
                setupHorizontal();
            }
            else
            {
                setupVertical();
            }
        }

        void setupVertical()
        {
            width = [this](){ return thickness.Get(); };
            height = [this](){ return m_view_widget->height.Get(); };

            m_grip->width = [this](){ return width.Get(); };

            m_grip->height =
                    [this](){
                        float ratio =
                                m_view_widget->height.Get()/
                                m_content_widget->height.Get();

                        return (std::min(1.0f,ratio)*m_view_widget->height.Get());
                    };

            m_grip->x = 0.0f;

            m_grip->y =
                    [this](){
                        float track_height = m_track->height.Get();
                        float full_range = track_height-m_grip->height.Get();
                        float content_y = m_content_widget->y.Get();
                        float content_h = m_content_widget->height.Get();

                        if(track_height >= content_h)
                        {
                            return 0.0f;
                        }
                        else
                        {
                            return ((content_y*-1.0f)/
                                    (content_h-track_height))*
                                    full_range;
                        }
                    };
        }

        void setupHorizontal()
        {
            height = [&](){ return thickness.Get(); };
            width = [&](){ return m_view_widget->width.Get(); };

            m_grip->height = [&](){ return height.Get(); };

            m_grip->width =
                    [this](){
                        float ratio =
                                m_view_widget->width.Get()/
                                m_content_widget->width.Get();

                        return (std::min(1.0f,ratio)*m_view_widget->width.Get());
                    };

            m_grip->y = 0.0f;

            m_grip->x =
                    [this](){
                        float track_width = m_track->width.Get();
                        float full_range = track_width-m_grip->width.Get();
                        float content_x = m_content_widget->x.Get();
                        float content_w = m_content_widget->width.Get();

                        if(track_width >= content_w)
                        {
                            return 0.0f;
                        }
                        else
                        {
                            return ((content_x*-1.0f)/
                                    (content_w-track_width))*
                                    full_range;
                        }
                    };
        }

        Widget* m_view_widget{nullptr};
        Widget* m_content_widget{nullptr};
        Widget* m_track{nullptr};
        Widget* m_grip{nullptr};

        Id m_cid_direction;
    };
}

#endif // RAINTK_SCROLL_BAR_HPP
