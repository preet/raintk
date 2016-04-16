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

#include <raintk/RainTkWidget.hpp>
#include <raintk/RainTkScene.hpp>
#include <raintk/RainTkTransformSystem.hpp>

#include <raintk/thirdparty/pnpoly.hpp>

namespace raintk
{
    ChildAlreadyExists::ChildAlreadyExists(std::string msg) :
        ks::Exception(ks::Exception::ErrorLevel::WARN,std::move(msg),true)
    {}

    ChildDoesNotExist::ChildDoesNotExist(std::string msg) :
        ks::Exception(ks::Exception::ErrorLevel::WARN,std::move(msg),true)
    {}

    // ============================================================= //

    Widget::Widget(ks::Object::Key const &key,
                   Scene* scene,
                   shared_ptr<Widget> parent) :
        ks::Object(key,scene->GetEventLoop()),
        m_scene(scene),
        m_parent(parent),
        m_cmlist_update_data(
            static_cast<UpdateDataComponentList*>(
                m_scene->template GetComponentList<UpdateData>())),
        m_cmlist_xf_data(
            static_cast<TransformDataComponentList*>(
                m_scene->template GetComponentList<TransformData>()))
    {}

    void Widget::Init(ks::Object::Key const &,
                      shared_ptr<Widget> const &this_widget)
    {
        // Set origin property
        origin.Bind([this](){
            auto const new_width = width.Get();
            auto const new_height = height.Get();

            return glm::vec2{
                (0.5f*new_width),
                (0.5f*new_height)
            };
        });

        // Add this widget as a child of its parent
        auto parent = m_parent.lock();
        if(parent) {
            parent->AddChild(this_widget);
        }

        // Create entity:
        m_entity_id = m_scene->CreateEntity();

        // (Transform component)
        glm::vec3 const position{
            x.Get(),
            y.Get(),
            z.Get()
        };

        m_cmlist_xf_data->Create(
                    m_entity_id,
                    TransformData{
                        position,
                        rotation.Get(),
                        scale.Get(),
                        origin.Get(),
                        false,
                        glm::mat4{1.0},
                        BoundingBox{}
                    });

        // (Update component)
        auto& update_data =
                m_cmlist_update_data->Create(
                    m_entity_id,
                    UpdateData());

        update_data.widget = this;

        // Queue an update for TransformData to calculate
        // the initial bounding box
        update_data.update |= UpdateData::UpdateTransform;


        // Connect Properties
        m_cid_width =
                width.signal_changed.Connect(
                    this_widget,
                    &Widget::onWidthChanged,
                    ks::ConnectionType::Direct);

        m_cid_height =
                height.signal_changed.Connect(
                    this_widget,
                    &Widget::onHeightChanged,
                    ks::ConnectionType::Direct);

        m_cid_x =
                x.signal_changed.Connect(
                    this_widget,
                    &Widget::onXChanged,
                    ks::ConnectionType::Direct);

        m_cid_y =
                y.signal_changed.Connect(
                    this_widget,
                    &Widget::onYChanged,
                    ks::ConnectionType::Direct);

        m_cid_z =
                z.signal_changed.Connect(
                    this_widget,
                    &Widget::onZChanged,
                    ks::ConnectionType::Direct);

        m_cid_rotation =
                rotation.signal_changed.Connect(
                    this_widget,
                    &Widget::onRotationChanged,
                    ks::ConnectionType::Direct);

        m_cid_scale =
                scale.signal_changed.Connect(
                    this_widget,
                    &Widget::onScaleChanged,
                    ks::ConnectionType::Direct);

        m_cid_origin =
                origin.signal_changed.Connect(
                    this_widget,
                    &Widget::onOriginChanged,
                    ks::ConnectionType::Direct);

        m_cid_clip =
                clip.signal_changed.Connect(
                    this_widget,
                    &Widget::onClipChanged,
                    ks::ConnectionType::Direct);
    }

    Widget::~Widget()
    {
        signal_destroying_widget.Emit(this);

        m_scene->RemoveEntity(m_entity_id);
    }

    Scene* Widget::GetScene() const
    {
        return m_scene;
    }

    shared_ptr<Widget> Widget::GetParent() const
    {
        return m_parent.lock();
    }

    Widget::ListChildren const & Widget::GetChildren() const
    {
        return m_list_children;
    }

    Id Widget::GetEntityId() const
    {
        return m_entity_id;
    }

    Id Widget::GetClipId() const
    {
        return m_clip_id;
    }

    bool Widget::GetIsDrawable() const
    {
        return false;
    }

    void Widget::SetClipId(Id clip_id)
    {
        m_clip_id = clip_id;
        this->onClipIdUpdated();
    }

    void Widget::AddChild(shared_ptr<Widget> const &child)
    {
        if(!(m_list_children.insert(child).second))
        {
            std::string const s =
                    "Widget "+name+": Child "+
                    child->name+" already exists";

            throw ChildAlreadyExists(s);
        }

        shared_ptr<Widget> this_widget =
                std::static_pointer_cast<Widget>(
                    shared_from_this());

        child->m_parent = this_widget;
    }

    void Widget::RemoveChild(shared_ptr<Widget> const &child)
    {       
        if(!(m_list_children.erase(child) > 0))
        {
            std::string const s =
                    "Widget "+name+": Child "+
                    child->name+" does not exist";

            throw ChildDoesNotExist(s);
        }

        child->m_parent.reset();
    }

    namespace
    {
        void GetDFSForWidget(Widget* parent,std::vector<Widget*>& list_widgets)
        {
            auto const &list_children = parent->GetChildren();
            for(auto& child : list_children)
            {
                GetDFSForWidget(child.get(),list_widgets);
            }

            list_widgets.push_back(parent);
        }
    }

    void Widget::UpdateHierarchy()
    {
        // Get a list of all widget entities in DFS order
        // TODO: Should we call this over and over until
        // all UpdateWidget flags are false?
        std::vector<Widget*> list_widgets;
        GetDFSForWidget(this,list_widgets);
        for(auto widget : list_widgets)
        {
            auto& upd_data =
                    m_cmlist_update_data->
                    GetComponent(widget->GetEntityId());

            if(upd_data.update & UpdateData::UpdateWidget)
            {
                widget->update();
                upd_data.update &= ~(UpdateData::UpdateWidget);
            }
        }
    }

    glm::vec2 Widget::CalcLocalCoords(Widget* widget,
                                      glm::vec2 const &world_point)
    {
        auto const &xf_data =
                widget->m_cmlist_xf_data->GetComponent(
                    widget->GetEntityId());

        // Transform the point so its in local coord
        auto const &xf = xf_data.world_xf;

        glm::vec2 local_point(
                    glm::inverse(xf)*
                    glm::vec4(world_point.x,world_point.y,0,1));

        return local_point;
    }

    glm::vec2 Widget::CalcWorldCoords(
                    Widget* widget,
                    glm::vec2 const &local_point)
    {
        auto const &xf_data =
                widget->m_cmlist_xf_data->GetComponent(
                    widget->GetEntityId());

        // Transform the point so its in world coords
        glm::vec2 world_point(
                    xf_data.world_xf*
                    glm::vec4(local_point.x,local_point.y,0,1));

        return world_point;
    }

    bool Widget::CalcPointInside(Widget* widget,
                                 glm::vec2 const &world_point)
    {
        auto const &xf_data =
                widget->m_cmlist_xf_data->GetComponent(
                    widget->GetEntityId());

        // First do an unclipped widget test

        // Transform the point so its in local coord
        glm::vec2 local_point = CalcLocalCoords(widget,world_point);

        bool const outside_widget =
                local_point.x < 0.0f || local_point.x > widget->width.Get() ||
                local_point.y < 0.0f || local_point.y > widget->height.Get();

        if(outside_widget)
        {
            return false;
        }


        // Point in poly test, takes clipping into account
        // poly_vx is in world coordinates
        auto const &poly_vx = xf_data.poly_vx;
        return CalcPointInPoly(poly_vx,world_point);
    }

    bool Widget::CalcPointInside(
                    Widget* widget,
                    glm::vec2 const &world_point,
                    glm::vec2 const &local_point)
    {
        auto const &xf_data =
                widget->m_cmlist_xf_data->GetComponent(
                    widget->GetEntityId());

        // First do an unclipped widget test
        bool const outside_widget =
                local_point.x < 0.0f || local_point.x > widget->width.Get() ||
                local_point.y < 0.0f || local_point.y > widget->height.Get();

        if(outside_widget)
        {
            return false;
        }

        // Point in poly test, takes clipping into account
        // poly_vx is in world coordinates
        auto const &poly_vx = xf_data.poly_vx;
        return CalcPointInPoly(poly_vx,world_point);
    }

    void Widget::onWidthChanged()
    {
        // Do nothing for base widget
    }

    void Widget::onHeightChanged()
    {
        // Do nothing for base widget
    }

    void Widget::onXChanged()
    {
        auto& xf_data = m_cmlist_xf_data->GetComponent(m_entity_id);
        xf_data.position.x = x.Get();

        auto& upd_data = m_cmlist_update_data->GetComponent(m_entity_id);
        upd_data.update |= UpdateData::UpdateTransform;
    }

    void Widget::onYChanged()
    {
        auto& xf_data = m_cmlist_xf_data->GetComponent(m_entity_id);
        xf_data.position.y = y.Get();

        auto& upd_data = m_cmlist_update_data->GetComponent(m_entity_id);
        upd_data.update |= UpdateData::UpdateTransform;
    }

    void Widget::onZChanged()
    {
        auto& xf_data = m_cmlist_xf_data->GetComponent(m_entity_id);
        xf_data.position.z = z.Get();

        auto& upd_data = m_cmlist_update_data->GetComponent(m_entity_id);
        upd_data.update |= UpdateData::UpdateTransform;
    }

    void Widget::onRotationChanged()
    {
        auto& xf_data = m_cmlist_xf_data->GetComponent(m_entity_id);
        xf_data.rotation = rotation.Get();

        auto& upd_data = m_cmlist_update_data->GetComponent(m_entity_id);
        upd_data.update |= UpdateData::UpdateTransform;
    }

    void Widget::onOriginChanged()
    {
        auto& xf_data = m_cmlist_xf_data->GetComponent(m_entity_id);
        xf_data.origin = origin.Get();

        auto& upd_data = m_cmlist_update_data->GetComponent(m_entity_id);
        upd_data.update |= UpdateData::UpdateTransform;
    }

    void Widget::onScaleChanged()
    {
        auto& xf_data = m_cmlist_xf_data->GetComponent(m_entity_id);
        xf_data.scale = scale.Get();

        auto& upd_data = m_cmlist_update_data->GetComponent(m_entity_id);
        upd_data.update |= UpdateData::UpdateTransform;
    }

    void Widget::onClipChanged()
    {
        auto& upd_data = m_cmlist_update_data->GetComponent(m_entity_id);
        upd_data.update |= UpdateData::UpdateClip;
    }

    void Widget::onClipIdUpdated()
    {
        // Do nothing for base widget
    }

    void Widget::onTransformUpdated()
    {
        // Do nothing for base widget
    }

    void Widget::update()
    {
        // Do nothing for base widget
    }
}
