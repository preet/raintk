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

#ifndef RAINTK_WIDGET_HPP
#define RAINTK_WIDGET_HPP

#include <unordered_set>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

#include <raintk/RainTkProperty.hpp>
#include <raintk/RainTkComponents.hpp>
#include <raintk/RainTkUnits.hpp>

namespace raintk
{
    // ============================================================= //

    class ChildAlreadyExists : public ks::Exception
    {
    public:
        ChildAlreadyExists(std::string msg);
        ~ChildAlreadyExists() = default;
    };

    class ChildDoesNotExist : public ks::Exception
    {
    public:
        ChildDoesNotExist(std::string msg);
        ~ChildDoesNotExist() = default;
    };

    // ============================================================= //

    class Scene;

    class Widget : public ks::Object
    {
    public:
        using base_type = ks::Object;
        using ListChildren = std::unordered_set<shared_ptr<Widget>>;

        friend class TransformSystem;

        // Only the Scene should be able to create a root widget
        class RootWidgetKey {
            friend class Scene;
            RootWidgetKey() {} // private constructor
        };


        // Note: These fields are placed at the top since they
        //       should be initialized before the properties
        //       so that property names are correct
        std::string name;


        Widget(ks::Object::Key const &key,
               Scene* scene,
               shared_ptr<Widget> parent);

        void Init(ks::Object::Key const &,
                  shared_ptr<Widget> const &);

        ~Widget();

        Scene* GetScene() const;
        shared_ptr<Widget> GetParent() const;
        ListChildren const &GetChildren() const;
        Id GetEntityId() const;       
        Id GetClipId() const;

        virtual bool GetIsDrawable() const;

        void SetClipId(Id clip_id);
        virtual void AddChild(shared_ptr<Widget> const &child);
        virtual void RemoveChild(shared_ptr<Widget> const &child);

        // Use this function to force an update() of this widget
        // and all of its children (children are updated before
        // parents). This is used when widget properties that
        // would be updated later are needed immediately.
        void UpdateHierarchy();


        // Helpers

        // Converts a point in world space to widget-local coords
        static glm::vec2 CalcLocalCoords(
                Widget* widget,
                glm::vec2 const &world_point);

        static glm::vec2 CalcWorldCoords(
                Widget* widget,
                glm::vec2 const &local_point);

        // Determines if @world_point (ie in world coordinate
        // space) is inside @widget
        static bool CalcPointInside(
                Widget* widget,
                glm::vec2 const &world_point);

        // Variant that won't calculate the local point if
        // its already available
        static bool CalcPointInside(
                Widget* widget,
                glm::vec2 const &world_point,
                glm::vec2 const &local_point);


        // Properties
        Property<float> width{
            0.0f
        };

        Property<float> height{
            0.0f
        };

        Property<float> x{
            0.0f
        };

        Property<float> y{
            0.0f
        };

        Property<float> z{
            0.0f
        };

        // The rotation angle in rads
        Property<float> rotation{
            0.0f
        };

        Property<glm::vec2> scale{
            glm::vec2{1.0f}
        };

        Property<glm::vec2> origin{
            glm::vec2{0.0f}
        };

        Property<bool> clip{
            false
        };

        Property<float> opacity{
            1.0f
        };

        // signal that is emitted from within the
        // destructor of this Widget
        ks::Signal<Widget*> signal_destroying_widget;

    protected:
        virtual void onWidthChanged();
        virtual void onHeightChanged();
        virtual void onXChanged();
        virtual void onYChanged();
        virtual void onZChanged();
        virtual void onRotationChanged();
        virtual void onScaleChanged();
        virtual void onOriginChanged();
        virtual void onClipChanged();

        virtual void onClipIdUpdated();
        virtual void onTransformUpdated();
        virtual void onAccOpacityUpdated();

        virtual void update();

        Scene* m_scene;
        weak_ptr<Widget> m_parent;
        ListChildren m_list_children;

        Id m_entity_id{0};
        UpdateDataComponentList* const m_cmlist_update_data;
        TransformDataComponentList* const m_cmlist_xf_data;

        Id m_cid_width;
        Id m_cid_height;
        Id m_cid_x;
        Id m_cid_y;
        Id m_cid_z;
        Id m_cid_rotation;
        Id m_cid_scale;
        Id m_cid_origin;
        Id m_cid_clip;
        Id m_cid_opacity;

        Id m_clip_id;

#ifdef RAINTK_TEST_OPACITY_HIERARCHY
    public:
#endif
        float m_accumulated_opacity;
    };

    // ============================================================= //

    template<typename T, typename... Args>
    shared_ptr<T> MakeWidget(Args&&... args)
    {
        // (std::is_base_of<Widget,Widget> is true as well)
        static_assert(std::is_base_of<Widget,T>::value,
                      "raintk::MakeWidget: Type must be Widget or"
                      "derive from Widget");

        return ks::MakeObject<T>(std::forward<Args>(args)...);
    }

    // ============================================================= //
}

#endif // RAINTK_WIDGET_HPP
