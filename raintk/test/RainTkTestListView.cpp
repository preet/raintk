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
#include <raintk/RainTkListModelSTLVector.hpp>
#include <raintk/RainTkListDelegate.hpp>
#include <raintk/RainTkListView.hpp>
#include <raintk/RainTkScrollArea.hpp>
#include <raintk/RainTkRow.hpp>
#include <raintk/RainTkText.hpp>
#include <raintk/RainTkSinglePointArea.hpp>
#include <raintk/RainTkRectangle.hpp>

// =========================================================== //
// =========================================================== //

namespace {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<uint> dis(2,8); // [2,8] // float r = mm(0.5*(dis(mt)));

    // some colors
    glm::u8vec4 g_color_yellow{242,209,91,255};
    glm::u8vec4 g_color_green{91,191,33,255};
    glm::u8vec4 g_color_blue{0,142,214,255};
    glm::u8vec4 g_color_purple{170,71,186,255};
    glm::u8vec4 g_color_red{239,53,45,255};
}

namespace raintk
{
    struct TestItem
    {
        glm::u8vec4 color;
    };

    class TestDelegate : public ListDelegate
    {
    public:
        TestDelegate(ks::Object::Key const &key,
                     Scene* scene,
                     shared_ptr<Widget> parent) :
            ListDelegate(key,scene,parent),
            m_index(0)
        {}

        void Init(ks::Object::Key const &,
                  shared_ptr<TestDelegate> const &this_delegate)
        {
            m_rect = MakeWidget<Rectangle>(m_scene,this_delegate);

            m_text = MakeWidget<Text>(m_scene,m_rect);
            m_text->font = "FiraSansMinimal.ttf";
            m_text->color = glm::u8vec4(255,255,255,255);
            m_text->z = m_rect->z.Get() + mm(1.0f);
            m_text->text = "Delegate";
            m_text->UpdateHierarchy(); // We need the final text height

            m_rect->width = this->GetParent()->width.Get();
            m_rect->height = m_text->height.Get() + mm(0.5*(dis(mt)));

            width = m_rect->width.Get();
            height = m_rect->height.Get();
        }

        ~TestDelegate()
        {}

        void SetIndex(uint index)
        {
            m_index = index;
            m_text->text = ks::ToString(m_index)+". Delegate";

            if(index==0)
            {
                m_rect->color = g_color_purple;
            }
            else
            {
                m_rect->color = m_color;
            }
        }

        uint GetIndex() const
        {
            return m_index;
        }

        void SetData(TestItem const &item)
        {
            m_color = item.color;
            if(m_color == glm::u8vec4(0,0,0,255))
            {
                auto new_height = m_rect->height.Get()*2;
                m_rect->height = new_height;
                height = new_height;

                m_rect->color = m_color;
            }
        }

    private:
        uint m_index;
        float m_width;
        shared_ptr<Rectangle> m_rect;
        shared_ptr<Text> m_text;

        glm::u8vec4 m_color;
    };
}

// =========================================================== //
// =========================================================== //

using namespace raintk;

std::vector<shared_ptr<SinglePointArea>>
CreateButtonRow(shared_ptr<Widget> root,
                float y_position)
{
    std::vector<shared_ptr<SinglePointArea>> list_sp_areas;

    auto scene = root->GetScene();

    auto row = MakeWidget<Row>(scene,root);

    row->height = mm(10);
    row->spacing = mm(2);
    row->y = y_position;

    std::vector<std::string> list_names{
        "+ Beg","+ Mid","+ End","- Beg","- Mid","- End",
        "resize 5"};

    auto create_button =
            [&](std::string button_name)
            {
                auto button_rect = MakeWidget<Rectangle>(scene,row);

                button_rect->width = mm(15);
                button_rect->height = mm(10);
                button_rect->z = mm(3.0f);

                auto button_text = MakeWidget<Text>(scene,button_rect);
                button_text->text = button_name;
                button_text->color = glm::u8vec4(255,255,255,255);
                button_text->z = button_rect->z.Get() + mm(1.0f);
                button_text->font = "FiraSansMinimal.ttf";
                button_text->size = mm(5);

                auto button_sp_area =
                        MakeWidget<SinglePointArea>(
                            scene,button_rect);

                button_sp_area->width = button_rect->width.Get();
                button_sp_area->height = button_rect->height.Get();
                return button_sp_area;
            };

    for(uint i=0; i < 3; i++)
    {
        auto button_sp_area = create_button(list_names[i]);
        list_sp_areas.push_back(button_sp_area);
    }

    for(uint i=3; i < 6; i++)
    {
        auto button_sp_area = create_button(list_names[i]);
        list_sp_areas.push_back(button_sp_area);
    }

    for(uint i=6; i < 7; i++)
    {
        auto button_sp_area = create_button(list_names[i]);
        list_sp_areas.push_back(button_sp_area);
    }


    return list_sp_areas;
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    TestContext c(600,800);
    auto scene = c.scene.get();
    auto root = c.scene->GetRootWidget();
//    c.scene->SetShowDebugText(false);

    // ListModel
    auto list_model = make_shared<ListModelSTLVector<TestItem>>();
    for(uint i=0; i < 25; i++)
    {
        list_model->PushBack(TestItem{glm::u8vec4{96,200,90,255}});
    }


    // Create add buttons
    auto list_button_sp_areas =
            CreateButtonRow(root,0.0f);

    std::vector<TestItem> list_add_insert{
//        TestItem{glm::u8vec3{249,155,12}},
        TestItem{glm::u8vec4{249,155,12,255}}
    };

    auto on_clicked_add_beg =
            [&](){
                list_model->Insert(
                            0,
                            list_add_insert);
            };

    auto on_clicked_add_mid =
            [&](){
                list_model->Insert(
                            list_model->GetSize()/2,
                            list_add_insert);
            };

    auto on_clicked_add_end =
            [&](){
                list_model->Insert(
                            list_model->GetSize(),
                            list_add_insert);
            };

    auto on_clicked_rem_beg =
            [&](){
                if(list_model->GetSize() > 0)
                {
                    list_model->Erase(0);
                }
            };

    auto on_clicked_rem_mid =
            [&](){
                if(list_model->GetSize() > 0)
                {
                    list_model->Erase(list_model->GetSize()/2);
                }
            };

    auto on_clicked_rem_end =
            [&](){
                if(list_model->GetSize() > 0)
                {
                    list_model->Erase(list_model->GetSize()-1);
                }
            };

    auto on_clicked_rsz_5 =
            [&](){
                if(list_model->GetSize() >= 5)
                {
                    list_model->SetData(4,TestItem{glm::u8vec4(0,0,0,255)});
                }
            };

    list_button_sp_areas[0]->signal_clicked.Connect(
                on_clicked_add_beg,
                nullptr,
                ks::ConnectionType::Direct);

    list_button_sp_areas[1]->signal_clicked.Connect(
                on_clicked_add_mid,
                nullptr,
                ks::ConnectionType::Direct);

    list_button_sp_areas[2]->signal_clicked.Connect(
                on_clicked_add_end,
                nullptr,
                ks::ConnectionType::Direct);

    list_button_sp_areas[3]->signal_clicked.Connect(
                on_clicked_rem_beg,
                nullptr,
                ks::ConnectionType::Direct);

    list_button_sp_areas[4]->signal_clicked.Connect(
                on_clicked_rem_mid,
                nullptr,
                ks::ConnectionType::Direct);

    list_button_sp_areas[5]->signal_clicked.Connect(
                on_clicked_rem_end,
                nullptr,
                ks::ConnectionType::Direct);

    list_button_sp_areas[6]->signal_clicked.Connect(
                on_clicked_rsz_5,
                nullptr,
                ks::ConnectionType::Direct);


    // Create list view
    auto list_view_bgbg =
            MakeWidget<Rectangle>(
                scene,root);

    list_view_bgbg->width = mm(60);
    list_view_bgbg->height = mm(150);
    list_view_bgbg->x = mm(10);
    list_view_bgbg->y = 0.5*(root->height.Get()-list_view_bgbg->height.Get());
    list_view_bgbg->z = mm(0.5);
    list_view_bgbg->color = glm::u8vec4(30,60,60,255);


    auto list_view_bg =
            MakeWidget<Rectangle>(
                scene,root);

    list_view_bg->width = mm(60);
    list_view_bg->height = mm(100);
    list_view_bg->x = mm(10);
    list_view_bg->y = 0.5*(root->height.Get()-list_view_bg->height.Get());
    list_view_bg->z = 1.0;
    list_view_bg->color = glm::u8vec4(60,60,60,255);


    // ListView
    auto list_view =
            MakeWidget<ListView<TestItem,TestDelegate>>(
                scene,list_view_bg);

    list_view->width = list_view_bg->width.Get();
    list_view->height = list_view_bg->height.Get();
    list_view->z = mm(1.5);
    list_view->spacing = mm(2.5);
    list_view->SetListModel(list_model);
    list_view->ShowDebugGuidelines(true);

    // Debug Lines
    auto line_top = MakeWidget<Rectangle>(scene,root);
    line_top->color = glm::u8vec4(255,255,255,255);
    line_top->height = mm(0.5);
    line_top->width = root->width.Get();
    line_top->y = list_view_bg->y.Get();

    auto line_bottom = MakeWidget<Rectangle>(scene,root);
    line_bottom->color = glm::u8vec4(255,255,255,255);
    line_bottom->height = mm(0.5);
    line_bottom->width = root->width.Get();
    line_bottom->y = list_view_bg->y.Get() + list_view_bg->height.Get();


    // Scrollbar track
    auto scroll_track =
            MakeWidget<Rectangle>(
                scene,list_view_bg);

    scroll_track->height =
            [&](){
                return list_view->height.Get();
            };
    scroll_track->width = mm(2);
    scroll_track->x =
            [&](){
                return (list_view->x.Get() + list_view->width.Get());
            };
    scroll_track->y =
            [&](){
                return list_view->y.Get();
            };
    scroll_track->color = glm::u8vec4{0,0,0,255};

    // Scrollbar grip
    auto scroll_grip =
            MakeWidget<Rectangle>(
                scene,scroll_track);

    scroll_grip->height =
            [&](){
//                rtklog.Trace() << "###: " << scroll_track->height.Get();
//                rtklog.Trace() << "###: " << list_view->height.Get();
//                rtklog.Trace() << "###: " << list_view->GetContentParent()->height.Get();
//                rtklog.Trace() << "*";

                float ratio =
                        std::min(
                            1.0f,
                            list_view->height.Get()/
                            list_view->GetContentParent()->height.Get());

                return scroll_track->height.Get()*ratio;
            };

    scroll_grip->width =
            [&](){
                return scroll_track->width.Get();
            };

    scroll_grip->y =
            [&](){
                float full_range =
                        scroll_track->height.Get()-
                        scroll_grip->height.Get();

                float content_parent_y =
                        list_view->GetContentParent()->y.Get();

                float content_parent_height =
                        list_view->GetContentParent()->height.Get();

                float track_height =
                        scroll_track->height.Get();

                if(track_height >= content_parent_height)
                {
                    return 0.0f;
                }
                else
                {
                    return ((content_parent_y*-1.0f)/
                           (content_parent_height-track_height))*
                           full_range;
                }
            };

    scroll_grip->z = mm(1.0f);


    // Content Parent Stats
    auto ctp_desc = MakeWidget<Text>(scene,root);
    ctp_desc->x = list_view_bgbg->x.Get() + list_view_bgbg->width.Get() + mm(10);
    ctp_desc->y = list_view_bgbg->y.Get() + mm(35);
    ctp_desc->color = glm::u8vec4(255,255,255,255);
    ctp_desc->text = "Content Parent";
    ctp_desc->font = "FiraSansMinimal.ttf";

    auto ctp_y = MakeWidget<Text>(scene,root);
    ctp_y->x = ctp_desc->x.Get();
    ctp_y->y = ctp_desc->y.Get() + mm(10);
    ctp_y->color = glm::u8vec4(255,255,255,255);
    ctp_y->text = [&](){
        auto y = list_view->GetContentParent()->y.Get();
        return std::string("Y: ")+ks::ToString(y);
    };
    ctp_y->font = "FiraSansMinimal.ttf";

    auto ctp_h = MakeWidget<Text>(scene,root);
    ctp_h->x = ctp_desc->x.Get();
    ctp_h->y = ctp_y->y.Get() + mm(10);
    ctp_h->color = glm::u8vec4(255,255,255,255);
    ctp_h->text = [&](){
        auto h = list_view->GetContentParent()->height.Get();
        return std::string("H: ")+ks::ToString(h);
    };
    ctp_h->font = "FiraSansMinimal.ttf";



    // Run!
    c.app->Run();

    return 0;
}
