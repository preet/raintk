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

#ifndef RAINTK_COMPONENTS_HPP
#define RAINTK_COMPONENTS_HPP

#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

#include <raintk/RainTkGlobal.hpp>
#include <raintk/RainTkDrawKey.hpp>
#include <raintk/RainTkSceneKey.hpp>

// Forward declarations
namespace ks
{
    namespace gl
    {
        class Texture2D;
    }

    namespace draw
    {
        template<typename SKT,typename DKT>
        class RenderSystem;

        class Geometry;

        template<typename DKT>
        class RenderData;

        enum class Transparency : u8;

        struct TextureSet;
    }

    namespace ecs
    {
        template<typename SKT, typename CT>
        class ComponentList;
    }
}

namespace raintk
{
    // ============================================================= //

    using RenderSystem =
        ks::draw::RenderSystem<SceneKey,DrawKey>;

    using Transparency =
        ks::draw::Transparency;

    using UPtrBuffer =
        unique_ptr<
            std::vector<u8>
        >;

    using AttrType = ks::gl::VertexBuffer::Attribute::Type;

    // ============================================================= //

    struct TextAtlasData
    {
        Id texture_set_id;
        shared_ptr<ks::draw::TextureSet> texture_set;

        ks::gl::Texture2D* atlas_texture;
    };

    // ============================================================= //

    // Bounding box for an entity
    // * Each entity with a TransformData component
    //   gets a corresponding BoundingBox
    // * The extents of the bounding box are determined
    //   from the associated Widget
    // * (x0,y0) represents the top left corner of the box
    //   (x1,y1) represents the bottom right corner of the box
    // * The coordinates are in world space
    struct BoundingBox
    {
        float x0;
        float y0;
        float x1;
        float y1;
    };

    // TODO We can optimize this struct
    // rotation, scale and origin are all 2d
    struct TransformData
    {
        glm::vec3 position; // 12
        float rotation; // 4 (angle in rads)
        glm::vec2 scale; // 12
        glm::vec2 origin; // 12

        bool valid; // 1

        glm::mat4 world_xf; // 64

        BoundingBox bbox; // 16
    };

    using TransformDataComponentList =
        ks::ecs::ComponentList<SceneKey,TransformData>;

    // ============================================================= //

    class Widget;
    struct UpdateData
    {
        static u8 const NoUpdates       = 0;
        static u8 const UpdateWidget    = (1 << 0);
        static u8 const UpdateTransform = (1 << 1);
        static u8 const UpdateDrawables = (1 << 2);

        UpdateData() :
            update(NoUpdates),
            widget(nullptr)
        {}

        u8 update;
        Widget* widget;
    };

    using UpdateDataComponentList =
        ks::ecs::ComponentList<SceneKey,UpdateData>;

    // ============================================================= //

    class InputArea;
    struct InputData
    {
        // TODO add Type: KeyInput,PointerInput
        bool enabled;
        InputArea* input_area;
    };

    // TODO: We should use another container (ie one that
    // isn't sparse) for InputData, because its likely only
    // a few entities will be input areas
    using InputDataComponentList =
        ks::ecs::ComponentList<SceneKey,InputData>;

    // ============================================================= //

    struct GeometryLayout
    {
        ks::gl::VertexLayout vx_layout;
        uint vx_size_bytes;
        uint buffer_size_hint_bytes;
    };

    struct DrawData
    {
        DrawKey key;
        UPtrBuffer vx_buffer;
        bool visible;
    };

    using DrawDataComponentList =
        ks::ecs::ComponentList<SceneKey,DrawData>;

    // ============================================================= //
}

#endif // RAINTK_COMPONENTS_HPP
