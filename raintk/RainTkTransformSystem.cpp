/*
   Copyright (C) 2015-2016 Preet Desai (preet.desai@gmail.com)

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

#include <raintk/RainTkAnimationSystem.hpp>
#include <raintk/RainTkTransformSystem.hpp>
#include <raintk/RainTkScene.hpp>
#include <raintk/RainTkWidget.hpp>
#include <raintk/RainTkLog.hpp>
#include <raintk/RainTkAnimation.hpp>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/string_cast.hpp>

#include <raintk/thirdparty/clipper/clipper.hpp>

namespace raintk
{
    TransformSystem::TransformSystem(Scene* scene) :
        m_scene(scene)
    {
        // Create the UpdateData component list
        m_scene->template RegisterComponentList<UpdateData>(
                    make_unique<UpdateDataComponentList>(*m_scene));

        m_cmlist_upd_data =
                static_cast<UpdateDataComponentList*>(
                    m_scene->template GetComponentList<UpdateData>());

        // Create the TransformData component list
        m_scene->template RegisterComponentList<TransformData>(
                    make_unique<TransformDataComponentList>(*m_scene));

        m_cmlist_xf_data =
                static_cast<TransformDataComponentList*>(
                    m_scene->template GetComponentList<TransformData>());
    }

    TransformSystem::~TransformSystem()
    {

    }

    std::string TransformSystem::GetDesc() const
    {
        return "raintk TransformSystem";
    }

    TransformDataComponentList*
    TransformSystem::GetTransformDataComponentList() const
    {
        return m_cmlist_xf_data;
    }

    void TransformSystem::Update(TimePoint const &/*prev_time*/,
                                 TimePoint const &/*curr_time*/)
    {
        updateLayout();
        updateTransforms();
        updateAnimations(); // must be after updateLayout
    }

    void TransformSystem::updateLayout()
    {
        // Handle requests for general widget updates in
        // multiple passes until all updates are satisfied
        // or we've reached some limit

        auto& list_upd_data = m_cmlist_upd_data->GetSparseList();

        uint const max_upd_loop_count=3;

        for(uint i=0; i < max_upd_loop_count; i++)
        {
            bool updated_widgets=false;

            // Ensure all queued signals are handled as they may
            // cause the UpdateData state of entities to change

            // Currently instead of doing this, we use a direct
            // connection for all property change notificiations
            // m_scene->GetEventLoop()->ProcessEvents();

            for(uint ent_id=0; ent_id < list_upd_data.size(); ent_id++)
            {
                auto& upd_data = list_upd_data[ent_id];

                if(upd_data.update & UpdateData::UpdateWidget)
                {
                    upd_data.widget->update();
                    upd_data.update &= ~(UpdateData::UpdateWidget);
                    updated_widgets = true;
                }
            }

            if(!updated_widgets)
            {
                break;
            }

            if(i == max_upd_loop_count-1)
            {
                // TODO throw?
                rtklog.Warn() << "UpdateSystem: "
                              << "Too many update loops with "
                                 "updated widgets still remaining";
            }
        }
    }

    void TransformSystem::updateTransforms()
    {
        // Update the Transform hierarchy using the Widget
        // parent/child tree
        updateWidgetTransforms(
                    m_scene->GetRootWidget().get(),
                    m_cmlist_upd_data,
                    m_cmlist_xf_data);

        updateWidgetClips(
                    m_scene->GetRootWidget().get(),
                    m_cmlist_upd_data,
                    m_cmlist_xf_data);
    }

    void TransformSystem::updateAnimations()
    {
        // TransformSystem::updateLayout must be completed in
        // between Animation::Start() and Animation::Update()
        // to ensure that up to date values are used for the
        // animation

        // If Animation::Start() is called during updateLayout,
        // nothing needs to be done. However if it is called
        // before or after updateLayout(), Animation::Update()
        // must be delayed until updateLayout() is called

        // To ensure this happens, we set the Animation's
        // m_ready flag to false in Animation::Start(), and
        // set it to true once updateLayout has completed

        // The AnimationSystem (which runs the following frame,
        // after TransformSystem) will only update Animations
        // that have m_ready set to true

        auto const &list_animations =
                m_scene->GetAnimationSystem()->
                GetListAnimations();

        for(Animation* animation : list_animations)
        {
            animation->m_ready = true;
        }
    }

    namespace
    {
        enum class GeometryResult : u8
        {
            XSEC_FALSE = 0,
            XSEC_TRUE,
            XSEC_COINCIDENT,
            XSEC_PARALLEL,
            XSEC_CONTAINED
        };

        // CalcLinesIntersect
        // * checks whether two 2d lines intersect and calculates
        //   the point of intersection (i_x1,i_y1)
        // * GeometryResult indicates whether the two lines intersect,
        //   are coincident, or parallel (if they don't intersect,
        //   i_x1 and i_y1 are invalid)
        GeometryResult CalcLineLineIntersection(glm::vec2 const &a1,
                                                glm::vec2 const &a2,
                                                glm::vec2 const &b1,
                                                glm::vec2 const &b2,
                                                glm::vec2 &xsec)
        {
            double ua_numr = (b2.x-b1.x)*(a1.y-b1.y)-(b2.y-b1.y)*(a1.x-b1.x);
            double ub_numr = (a2.x-a1.x)*(a1.y-b1.y)-(a2.y-a1.y)*(a1.x-b1.x);
            double denr = (b2.y-b1.y)*(a2.x-a1.x)-(b2.x-b1.x)*(a2.y-a1.y);

            if(denr == 0.0)
            {
                // lines are coincident
                if(ua_numr == 0.0 && ub_numr == 0.0)
                {   return GeometryResult::XSEC_COINCIDENT;   }

                // lines are parallel
                else
                {   return GeometryResult::XSEC_PARALLEL;   }
            }

            double ua = ua_numr/denr;
            double ub = ub_numr/denr;

            if(ua >= 0.0 && ua <= 1.0 && ub >= 0.0 && ub <= 1.0)
            {
                xsec.x = a1.x+ua*(a2.x-a1.x);
                xsec.y = a1.y+ua*(a2.y-a1.y);
                return GeometryResult::XSEC_TRUE;
            }

            return GeometryResult::XSEC_FALSE;
        }

        // * calculates whether or not test is to the left
        //   of the infinite line defined by p1 and p0
        // * returns > 0 if its to the left,
        //   returns = 0 if its on the line,
        //   returns < 0 if its to the right
        float CalcIsLeft(glm::vec2 p0, glm::vec2 p1, glm::vec2 test)
        {
            // 2d cross product
            return (p1.x - p0.x)*(test.y - p0.y) -
                    (test.x - p0.x)*(p1.y - p0.y);
        }

        // subj is clipped against clip
        // expect both to be CCW order
        std::vector<glm::vec2> g_poly_temp;

        // * Calculate the result of clipping @poly_subj against @poly_clip
        // * Expects CCW polygons
        // * No special consideration given to numerical robustness
        void CalcPolyClipSutherlandHodgman(
                std::array<glm::vec2,4> const &poly_subj,
                std::vector<glm::vec2> const &poly_clip,
                std::vector<glm::vec2>& poly_xsec)
        {
            // ref: https://www.cs.helsinki.fi/group/goa/viewing/leikkaus/intro2.html

            uint const poly_clip_sz = poly_clip.size();

            std::vector<glm::vec2> &poly_temp = g_poly_temp;
            poly_temp.clear();

            for(auto const &vx : poly_subj)
            {
                poly_temp.push_back(vx);
            }

            // For each clipping edge
            for(uint i=0; i < poly_clip_sz; i++)
            {
                auto const &clipping_edge_s = poly_clip[i];
                auto const &clipping_edge_e = poly_clip[(i+1)%poly_clip_sz];

                // For each subject edge
                // Clip poly_temp against the current edge of poly_clip,
                // forming a new polygon. Store the new polygon in poly_xsec
                poly_xsec.clear();
                uint const poly_temp_size = poly_temp.size();
                for(uint j=0; j < poly_temp_size; j++)
                {
                    auto const &subj_edge_s = poly_temp[j];
                    auto const &subj_edge_e = poly_temp[(j+1)%poly_temp_size];

                    // NOTE:
                    // CalcIsLeft assumes y increases +ve upwards, but our coord
                    // system has y increasing +ve downwards. This means we need
                    // to check if points are to the *right* of an edge to see
                    // if they are inside the polygon

                    if(CalcIsLeft(clipping_edge_s,clipping_edge_e,subj_edge_s) <= 0.0f)
                    {
                        if(CalcIsLeft(clipping_edge_s,clipping_edge_e,subj_edge_e) <= 0.0f)
                        {
                            // Case: Subject edge inside clip poly
                            poly_xsec.push_back(subj_edge_e);
                        }
                        else
                        {
                            // Case: Subject edge leaving clip poly
                            // TODO numerical robustness
                            glm::vec2 xsec = subj_edge_s;
                            CalcLineLineIntersection(
                                        clipping_edge_s,
                                        clipping_edge_e,
                                        subj_edge_s,
                                        subj_edge_e,
                                        xsec);

                            poly_xsec.push_back(xsec);
                        }
                    }
                    else
                    {
                        if(CalcIsLeft(clipping_edge_s,clipping_edge_e,subj_edge_e) <= 0.0f)
                        {
                            // Case: Subject edge entering clip poly
                            glm::vec2 xsec = subj_edge_s;
                            CalcLineLineIntersection(
                                        clipping_edge_s,
                                        clipping_edge_e,
                                        subj_edge_s,
                                        subj_edge_e,
                                        xsec);

                            poly_xsec.push_back(xsec);
                            poly_xsec.push_back(subj_edge_e);
                        }
                        else
                        {
                            // Case: Subject edge outside clip poly
                            // (save nothing)
                        }
                    }
                }

                // Save the new poly
                poly_temp = poly_xsec;
            }

            if(poly_xsec.size() < 3)
            {
                // The result is degenerate/incorrect
                poly_xsec.clear();
            }

        }


        ClipperLib::Clipper g_clipper;

        // * Calculate the polygon intersection between @poly_a
        //   and @poly_b (expect points in CCW order)
        void CalcPolyIntersection(std::array<glm::vec2,4> const &poly_a,
                                  std::vector<glm::vec2> const &poly_b,
                                  std::vector<glm::vec2>& poly_xsec)
        {
            if(poly_a.empty() || poly_b.empty())
            {
                return;
            }

            ClipperLib::Path clipper_poly_a;
            clipper_poly_a.reserve(poly_a.size());
            for(auto const &vx : poly_a)
            {
                clipper_poly_a.emplace_back(
                            static_cast<ClipperLib::cInt>(vx.x*1000.0f),
                            static_cast<ClipperLib::cInt>(vx.y*1000.0f));
            }

            ClipperLib::Path clipper_poly_b;
            clipper_poly_b.reserve(poly_b.size());
            for(auto const &vx : poly_b)
            {
                clipper_poly_b.emplace_back(
                            static_cast<ClipperLib::cInt>(vx.x*1000.0f),
                            static_cast<ClipperLib::cInt>(vx.y*1000.0f));
            }

            ClipperLib::Paths result;

            g_clipper.Clear();
            g_clipper.AddPath(clipper_poly_a,ClipperLib::ptSubject,true);
            g_clipper.AddPath(clipper_poly_b,ClipperLib::ptClip,true);
            if(!g_clipper.Execute(ClipperLib::ctIntersection,result))
            {
                // TODO Throw?
                rtklog.Trace() << "InputSystem: clipper Intersection failed";
            }

            if(result.size() == 1)
            {
                auto const &poly_result = result[0];
                poly_xsec.reserve(poly_result.size());
                for(auto const &vx : poly_result)
                {
                    poly_xsec.push_back(
                                glm::vec2(vx.X*0.001f,
                                          vx.Y*0.001f));
                }
            }

            // If the result size is > 1, something went wrong
            // so ignore the result

            // If the result size is 0, there's no intersection
            // (this is a valid result)
        }

        void SetRootTransformData(Widget* root,TransformData& xf_data)
        {
            auto root_width = root->width.Get();
            auto root_height = root->height.Get();

            xf_data.valid = true;
            xf_data.world_xf = glm::mat4(1.0f); // identity
            xf_data.bbox =
                    BoundingBox{
                        0.0f,
                        0.0f,
                        root_width,
                        root_height
                    };

            xf_data.list_vx[0] = glm::vec2(0.0f, 0.0f);
            xf_data.list_vx[1] = glm::vec2(0.0f, root_height);
            xf_data.list_vx[2] = glm::vec2(root_width, root_height);
            xf_data.list_vx[3] = glm::vec2(root_width, 0.0f);

            xf_data.poly_vx.resize(4);
            xf_data.poly_vx[0] = xf_data.list_vx[0];
            xf_data.poly_vx[1] = xf_data.list_vx[1];
            xf_data.poly_vx[2] = xf_data.list_vx[2];
            xf_data.poly_vx[3] = xf_data.list_vx[3];
        }

        struct UpdXFStackFrame
        {
            Widget* widget;
            u8 visit; // 0: not visited, 1: visited
            glm::mat4* parent_xf;
        };

        struct UpdClipStackFrame
        {
            Widget* widget;
            u8 visit;
        };
    }

    void TransformSystem::updateWidgetClips(
            Widget* root,
            UpdateDataComponentList* cmlist_upd_data,
            TransformDataComponentList* cmlist_xf_data)
    {

        // Create the depth first traversal stack
        std::vector<UpdClipStackFrame> dfs_stack;
        dfs_stack.push_back(
                    UpdClipStackFrame{
                        root,0
                    });

        // Create the clip stack
        std::vector<std::vector<glm::vec2>*> clip_stack;

        // The root's poly_vx has already been filled
        // by updateTransforms()

        auto& root_xf_data =
                cmlist_xf_data->GetComponent(
                    root->GetEntityId());

        auto clip_stack_base = root_xf_data.poly_vx;
        clip_stack.push_back(&clip_stack_base);


        // Traverse the widget tree
        while(!dfs_stack.empty())
        {
            if(dfs_stack.back().visit == 0)
            {
                // Visit
                Widget* widget = dfs_stack.back().widget;
                auto const has_clip = widget->clip.Get();
                auto const ent_id = widget->GetEntityId();
                auto& xf_data = cmlist_xf_data->GetComponent(ent_id);
                auto& upd_data = cmlist_upd_data->GetComponent(ent_id);

                if(upd_data.update & UpdateData::UpdateClip)
                {
                    // Clip widget against stack top
                    xf_data.poly_vx.clear();

                    CalcPolyIntersection(
                                xf_data.list_vx,
                                *(clip_stack.back()),
                                xf_data.poly_vx);

                    upd_data.update &= ~(UpdateData::UpdateClip);

                    // Mark child clips as updated
                    for(auto &child : widget->GetChildren())
                    {
                        auto& upd_data = cmlist_upd_data->
                                GetComponent(child->GetEntityId());

                        upd_data.update |= UpdateData::UpdateClip;
                    }
                }

                // If this widget should clip its children, add
                // this widget's polygon to the clip stack
                if(has_clip)
                {
                    clip_stack.push_back(&(xf_data.poly_vx));
                }

                dfs_stack.back().visit = 1;

                // Schedule visit to children
                for(auto &child : widget->GetChildren())
                {
                    dfs_stack.push_back(
                                UpdClipStackFrame{
                                    child.get(),0,
                                });
                }
            }
            else
            {
                // Leave
                Widget* widget = dfs_stack.back().widget;
                auto const has_clip = widget->clip.Get();
                if(has_clip)
                {
                    clip_stack.pop_back();
                }

                dfs_stack.pop_back();
            }
        }
    }

    void TransformSystem::updateWidgetTransforms(
            Widget* root,
            UpdateDataComponentList* cmlist_upd_data,
            TransformDataComponentList* cmlist_xf_data)
    {
        // Fill out the correct TransformData for the root
        auto& root_xf_data =
                cmlist_xf_data->GetComponent(
                    root->GetEntityId());

        SetRootTransformData(root,root_xf_data);

        // Create the depth first traversal stack
        std::vector<UpdXFStackFrame> dfs_stack;
        dfs_stack.push_back(
                    UpdXFStackFrame{
                        root,0,&(root_xf_data.world_xf)
                    });


        // Traverse the widget tree
        while(!dfs_stack.empty())
        {
            if(dfs_stack.back().visit == 0)
            {
                // Visit
                Widget* widget = dfs_stack.back().widget;

                auto const ent_id = widget->GetEntityId();
                auto& xf_data = cmlist_xf_data->GetComponent(ent_id);
                auto& update_data = cmlist_upd_data->GetComponent(ent_id);

                // Update transform
                if(update_data.update & UpdateData::UpdateTransform)
                {
                    auto const &parent_xf = *(dfs_stack.back().parent_xf);

                    // Recalculate the world transform
                    glm::vec3 const xf_data_origin(xf_data.origin,0.0f);

                    xf_data.world_xf =
                            parent_xf *
                            glm::translate(xf_data.position) *
                            glm::translate(xf_data_origin) *
                            glm::rotate(xf_data.rotation,glm::vec3(0.0f,0.0f,1.0f)) *
                            glm::scale(glm::vec3(xf_data.scale,1.0f)) *
                            glm::translate(xf_data_origin*-1.0f);

                    widget->onTransformUpdated();
                    update_data.update &= ~(UpdateData::UpdateTransform);
                    xf_data.valid = true;

                    // Update the world coordinates and bounding box
                    auto const width = widget->width.Get();
                    auto const height = widget->height.Get();

                    xf_data.list_vx[0] = glm::vec2(xf_data.world_xf*glm::vec4(0.0f,0.0f,0.0f,1.0f));
                    xf_data.list_vx[1] = glm::vec2(xf_data.world_xf*glm::vec4(0.0f,height,0.0f,1.0f));
                    xf_data.list_vx[2] = glm::vec2(xf_data.world_xf*glm::vec4(width,height,0.0f,1.0f));
                    xf_data.list_vx[3] = glm::vec2(xf_data.world_xf*glm::vec4(width,0.0f,0.0f,1.0f));

                    xf_data.bbox.x0 = xf_data.list_vx[0].x;
                    xf_data.bbox.x1 = xf_data.list_vx[0].x;
                    xf_data.bbox.y0 = xf_data.list_vx[0].y;
                    xf_data.bbox.y1 = xf_data.list_vx[0].y;

                    for(uint i=1; i < 4; i++)
                    {
                        xf_data.bbox.x0 = std::min(xf_data.bbox.x0, xf_data.list_vx[i].x);
                        xf_data.bbox.x1 = std::max(xf_data.bbox.x1, xf_data.list_vx[i].x);
                        xf_data.bbox.y0 = std::min(xf_data.bbox.y0, xf_data.list_vx[i].y);
                        xf_data.bbox.y1 = std::max(xf_data.bbox.y1, xf_data.list_vx[i].y);
                    }

                    // Schedule a clip update
                    update_data.update |= UpdateData::UpdateClip;

                    // Mark all child transforms as requiring updates
                    for(auto& child : widget->GetChildren())
                    {
                        auto& upd_data =
                                cmlist_upd_data->
                                GetComponent(child->GetEntityId());

                        upd_data.update |= UpdateData::UpdateTransform;
                    }
                }

                dfs_stack.back().visit = 1;

                // Schedule visit to children
                for(auto &child : widget->GetChildren())
                {
                    dfs_stack.push_back(
                                UpdXFStackFrame{
                                    child.get(),0,&(xf_data.world_xf)
                                });
                }
            }
            else
            {
                // Leave
                dfs_stack.pop_back();
            }
        }
    }
}

