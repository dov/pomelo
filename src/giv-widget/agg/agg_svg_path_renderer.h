//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.3
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://www.antigrain.com
//----------------------------------------------------------------------------
// Gunnar Roth: add support for linear and radial gradients(support xlink attr),
// shape gradient opaqueness, rounded rects, circles,ellipses. support a command (arc)  in pathes. 
// set new origin correctly to last postion on z command in a path( was set to 0,0 before).
// enable parsing of colors written as rgb()
// some code was inspired by code from Haiku OS
/*
* Copyright 2006-2007, Haiku. All rights reserved.
* Distributed under the terms of the MIT License.
*
* Authors:
* Stephan Aßmus <superstippi@gmx.de>
+*/
//----------------------------------------------------------------------------
//
//
// SVG path renderer.
//
//----------------------------------------------------------------------------
#ifndef AGG_SVG_PATH_RENDERER_INCLUDED
#define AGG_SVG_PATH_RENDERER_INCLUDED

#include "agg_path_storage.h"
#include "agg_conv_transform.h"
#include "agg_conv_stroke.h"
#include "agg_conv_contour.h"
#include "agg_conv_curve.h"
#include "agg_color_rgba.h"
#include "agg_renderer_scanline.h"
#include "agg_bounding_rect.h"
#include "agg_ellipse.h"
#include "agg_rounded_rect.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_svg_path_tokenizer.h"
#include <vector>
#include "agg_svg_gradient.h"
#include "agg_span_gradient.h"
#include "agg_span_allocator.h"
#include "agg_span_interpolator_linear.h"
#include "agg_rounded_rect.h"

namespace agg
{
const double SVG_MM_TO_POINT = 2.834645651435303;

namespace svg
{
    inline rgba8 modify_color(const rgba8& color, double gain, double offset)
    {
      double rr = gain * color.r + offset;
      if (rr < 0)
        rr = 0;
      if (rr > 255)
        rr = 255;
      double gg = gain * color.g + offset;
      if (gg < 0)
        gg = 0;
      if (gg > 255)
        gg = 255;
      double bb = gain * color.b + offset;
      if (bb < 0)
        bb = 0;
      if (bb > 255)
        bb = 255;
      return rgba8(int(rr),int(gg),int(bb),color.a);
    }

    template<class VertexSource> class conv_count
    {
    public:
        conv_count(VertexSource& vs) : m_source(&vs), m_count(0) {}

        void count(unsigned n) { m_count = n; }
        unsigned count() const { return m_count; }

        void rewind(unsigned path_id) { m_source->rewind(path_id); }
        unsigned vertex(double* x, double* y) 
        { 
            ++m_count; 
            return m_source->vertex(x, y); 
        }

    private:
        VertexSource* m_source;
        unsigned m_count;
    };




    //============================================================================
    // Basic path attributes
    struct path_attributes
    {
        unsigned     index;
        rgba8        fill_color;
        rgba8        stroke_color;
        double       opacity;
        bool         fill_flag;
        bool         stroke_flag;
        bool         even_odd_flag;
        line_join_e  line_join;
        line_cap_e   line_cap;
        double       miter_limit;
        double       stroke_width;
        trans_affine transform;
 
        char	stroke_url[64];
        char	fill_url[64];

        // Empty constructor
        path_attributes() :
            index(0),
            fill_color(rgba(0,0,0)),
            stroke_color(rgba(0,0,0)),
            opacity(1.0),
            fill_flag(true),
            stroke_flag(false),
            even_odd_flag(true),
            line_join(miter_join),
            line_cap(butt_cap),
            miter_limit(4.0),
            stroke_width(1.0),
            transform()
        {
           stroke_url[0] = 0;
           fill_url[0] = 0;
        }

        // Copy constructor
        path_attributes(const path_attributes& attr) :
            index(attr.index),
            fill_color(attr.fill_color),
            stroke_color(attr.stroke_color),
            opacity(attr.opacity),
            fill_flag(attr.fill_flag),
            stroke_flag(attr.stroke_flag),
            even_odd_flag(attr.even_odd_flag),
            line_join(attr.line_join),
            line_cap(attr.line_cap),
            miter_limit(attr.miter_limit),
            stroke_width(attr.stroke_width),
            transform(attr.transform)
        {
           sprintf(stroke_url, "%s", attr.stroke_url);
           sprintf(fill_url, "%s", attr.fill_url);
        }

        // Copy constructor with new index value
        path_attributes(const path_attributes& attr, unsigned idx) :
            index(idx),
            fill_color(attr.fill_color),
            stroke_color(attr.stroke_color),
            fill_flag(attr.fill_flag),
            stroke_flag(attr.stroke_flag),
            even_odd_flag(attr.even_odd_flag),
            line_join(attr.line_join),
            line_cap(attr.line_cap),
            miter_limit(attr.miter_limit),
            stroke_width(attr.stroke_width),
            transform(attr.transform)
        {
           sprintf(stroke_url, "%s", attr.stroke_url);
           sprintf(fill_url, "%s", attr.fill_url);
        }
    };


    //============================================================================
    // Path container and renderer. 
    class path_renderer
    {
    public:
        typedef pod_bvector<path_attributes>   attr_storage;

        typedef conv_curve<path_storage>       curved;
        typedef conv_count<curved>             curved_count;

        typedef conv_stroke<curved_count>      curved_stroked;
        typedef conv_transform<curved_stroked> curved_stroked_trans;

        typedef conv_transform<curved_count>   curved_trans;
        typedef conv_contour<curved_trans>     curved_trans_contour;
    
        path_renderer();
        ~path_renderer()
        {
            for(size_t i = 0; i < m_gradients.size(); i++)
              delete m_gradients[i];
        }
        path_renderer(const path_renderer& other) :
          m_storage(other.m_storage),
          m_attr_storage(other.m_attr_storage),
          m_attr_stack(other.m_attr_stack),
          m_transform(other.m_transform),
          m_user_transform(other.m_user_transform),

          m_curved(m_storage),
          m_curved_count(m_curved),
  
          m_curved_stroked(m_curved_count),
          m_curved_stroked_trans(m_curved_stroked, m_transform),
  
          m_curved_trans(m_curved_count, m_transform),
          m_curved_trans_contour(m_curved_trans),
          m_paint_by_label(false)
        {
          for (size_t i=0; i<other.m_gradients.size(); i++)
                m_gradients.push_back(other.m_gradients[i]->clone());
        }

        const path_renderer& operator=(const path_renderer& other)
        {
            m_storage = other.m_storage;
            m_attr_storage = other.m_attr_storage;
            m_attr_stack = other.m_attr_stack;
            m_transform = other.m_transform; 
            m_user_transform = other.m_user_transform;
            for (size_t i=0; i<other.m_gradients.size(); i++)
                m_gradients.push_back(other.m_gradients[i]->clone());

            return *this;
        }

        void remove_all();

        // Use these functions as follows:
        // begin_path() when the XML tag <path> comes ("start_element" handler)
        // parse_path() on "d=" tag attribute
        // end_path() when parsing of the entire tag is done.
        void begin_path();
        void parse_path(path_tokenizer& tok);
        void end_path();

        // The following functions are essentially a "reflection" of
        // the respective SVG path commands.
        void move_to(double x, double y, bool rel=false);   // M, m
        void line_to(double x,  double y, bool rel=false);  // L, l
        void hline_to(double x, bool rel=false);            // H, h
        void vline_to(double y, bool rel=false);            // V, v
        void curve3(double x1, double y1,                   // Q, q
                    double x,  double y, bool rel=false);
        void curve3(double x, double y, bool rel=false);    // T, t
        void curve4(double x1, double y1,                   // C, c
                    double x2, double y2, 
                    double x,  double y, bool rel=false);
        void curve4(double x2, double y2,                   // S, s
                    double x,  double y, bool rel=false);
        void elliptical_arc(double rx, double ry,
                            double angle,
                            bool large_arc_flag,
                            bool sweep_flag,
                            double x, double y,
                            bool rel = false);	// A, a
        void roundrect(double x1, double y1,                   // C, c
                       double x2, double y2,double rx, double ry,bool rel = false)
        {
            if(rel)
            {
                m_storage.rel_to_abs(&x1, &y1);
                m_storage.rel_to_abs(&x2, &y2);
                
            }
            agg::rounded_rect rc;
            rc.rect(x1, y1, x2, y2);
            rc.radius(rx,ry);
            rc.normalize_radius();
            m_storage.concat_path(rc,0);
        }

        void arc_to(double rx, double ry,                   // A,a
                    double xrot,
                    bool large_arc,
                    bool sweep_arc,
                    double x,
                    double y,
                    bool rel);

        template<class VertexSource>
        void concat_path(VertexSource &vs)
        {
            m_storage.concat_path(vs);
        }
        void close_subpath();                               // Z, z

//        template<class VertexSource> 
//        void add_path(VertexSource& vs, 
//                      unsigned path_id = 0, 
//                      bool solid_path = true)
//        {
//            m_storage.add_path(vs, path_id, solid_path);
//        }


        unsigned vertex_count() const { return m_curved_count.count(); }
        

        // Call these functions on <g> tag (start_element, end_element respectively)
        void push_attr();
        void pop_attr();

        // Attribute setting functions.
        void fill(const rgba8& f);
        void stroke(const rgba8& s);
        void even_odd(bool flag);
        void stroke_width(double w);
        void fill_none();
        void fill_url(const char* url);
        void stroke_none();
        void stroke_url(const char* url);
        void opacity(double op);
        void fill_opacity(double op);
        void stroke_opacity(double op);
        void line_join(line_join_e join);
        void line_cap(line_cap_e cap);
        void miter_limit(double ml);
        trans_affine& transform();

        // Make all polygons CCW-oriented
        void arrange_orientations()
        {
            m_storage.arrange_orientations_all_paths(path_flags_ccw);
        }

        // Expand all polygons 
        void expand(double value)
        {
            m_curved_trans_contour.width(value);
        }

        unsigned operator [](unsigned idx)
        {
            m_transform = m_attr_storage[idx].transform;
            return m_attr_storage[idx].index;
        }

        void bounding_rect(double* x1, double* y1, double* x2, double* y2)
        {
            agg::conv_transform<agg::path_storage> trans(m_storage, m_transform);
            agg::bounding_rect(trans, *this, 0, m_attr_storage.size(), x1, y1, x2, y2);
        }

        template<class Rasterizer, class Scanline, class RendererBase,class GradientFunction>
        void render_gradient(Rasterizer& ras, 
            Scanline& sl,
            RendererBase& rb, const trans_affine& mtx, 
            GradientFunction gradient_func,
                             agg::svg::gradient_lut_opaque<agg::color_interpolator<typename RendererBase::color_type>, 256u>& lut,
                             int start,int end)
        {
            typedef agg::span_interpolator_linear<> interpolator_type;
            interpolator_type     span_interpolator(mtx);

            typedef agg::span_gradient<typename RendererBase::color_type, 
                interpolator_type, 
                GradientFunction,
                agg::svg::gradient_lut_opaque<agg::color_interpolator<typename RendererBase::color_type>, 256u>
                > span_gradient_type;

            span_gradient_type span_gradient(span_interpolator, 
                                             gradient_func, 
                                             lut, 
                                             start, end);

            typedef agg::span_allocator<typename RendererBase::color_type> span_allocator_type;
            span_allocator_type alloc;

            agg::render_scanlines_aa(ras, sl, rb, alloc, span_gradient);
        }

        template<class Rasterizer, class Scanline, class RendererBase>
        void render_gradient(Rasterizer& ras, 
            Scanline& sl,
            RendererBase& rb, const char * gradient_url,double opaque)
        {
            if (gradient* g = find_gradient(gradient_url)) 
            {
                gradient* gl = NULL;
                if(g->count_colors() == 0)
                {
                    const std::string & xlink = g->xlink();
                    gl = find_gradient(xlink.c_str() + 1);
                }
                trans_affine mtxgr = g->transform();
                mtxgr.multiply(m_transform);
                mtxgr.invert();
                if(gl)
                    gl->set_opaque(opaque);
                else
                    g->set_opaque(opaque);
                agg::svg::gradient_lut_opaque<agg::color_interpolator<typename RendererBase::color_type>, 256u> lut = gl ? gl->lut() : g->lut(); 
                if(g->type() == gradient::GRADIENT_CIRCULAR)
                {
                    gradient_circle    gradient_func;
                    render_gradient(ras,sl,rb,mtxgr,gradient_func,lut, 0, gradient::lut_range);
                }
                else if(g->type() == gradient::GRADIENT_LINEAR)
                {
                    gradient_x    gradient_func;
                    render_gradient(ras,sl,rb,mtxgr,gradient_func,lut,-gradient::lut_range,gradient::lut_range);                                
                }
            }
        }

        // Rendering. One can specify two additional parameters: 
        // trans_affine and opacity. They can be used to transform the whole
        // image and/or to make it translucent.
        template<class Rasterizer, class Scanline, class RendererBase> 
        void render(Rasterizer& ras, 
                    Scanline& sl,
                    RendererBase& rb, 
                    const trans_affine& mtx, 
                    const rect_i& cb, // clipbox
                    double opacity=1.0,
                    double color_multiplier=1.0,
                    double color_offset=1.0)
        {
            unsigned i;
            typedef agg::renderer_scanline_aa_solid<RendererBase> renderer_solid;

            renderer_solid ren(rb);
            ras.clip_box(cb.x1, cb.y1, cb.x2, cb.y2);
            m_curved_count.count(0);
            trans_affine umtx = m_user_transform;
            umtx *= mtx;

            for(i = 0; i < m_attr_storage.size(); i++)
            {
                const path_attributes& attr = m_attr_storage[i];
                m_transform = attr.transform;
                m_transform *= umtx;
                double scl = m_transform.scale();
                //m_curved.approximation_method(curve_inc);
                m_curved.approximation_scale(scl);
                m_curved.angle_tolerance(0.0);

                rgba8 color;

                if(attr.fill_flag)
                {
                    ras.reset();
                    ras.filling_rule(attr.even_odd_flag ? fill_even_odd : fill_non_zero);
                    if(fabs(m_curved_trans_contour.width()) < 0.0001)
                    {
                        ras.add_path(m_curved_trans, attr.index);
                    }
                    else
                    {
                        m_curved_trans_contour.miter_limit(attr.miter_limit);
                        ras.add_path(m_curved_trans_contour, attr.index);
                    }

                    if(attr.fill_url[0] != 0)
                    {
                        if (m_paint_by_label)
                          agg::render_scanlines_bin_solid(ras, sl, rb, m_label_color);
                        else
                          render_gradient(ras,sl,rb,attr.fill_url,attr.opacity);
                    }
                    else
                    {
                        if (m_paint_by_label) {
                            agg::render_scanlines_bin_solid(ras, sl, rb, m_label_color);
                        }
                        else {
                            if (color_multiplier != 1.0)
                              color = modify_color(attr.fill_color, color_multiplier, color_offset);
                            else
                              color = attr.fill_color;
    
                            color.opacity(color.opacity() * opacity*attr.opacity);
    
                            ren.color(color);
                            agg::render_scanlines(ras, sl, ren);
                        }
                    }
                }

                if(attr.stroke_flag)
                {
                    m_curved_stroked.width(attr.stroke_width);
                    //m_curved_stroked.line_join((attr.line_join == miter_join) ? miter_join_round : attr.line_join);
                    m_curved_stroked.line_join(attr.line_join);
                    m_curved_stroked.line_cap(attr.line_cap);
                    m_curved_stroked.miter_limit(attr.miter_limit);
                    m_curved_stroked.inner_join(inner_round);
                    m_curved_stroked.approximation_scale(scl);

                    // If the *visual* line width is considerable we 
                    // turn on processing of curve cusps.
                    //---------------------
                    if(attr.stroke_width * scl > 1.0)
                    {
                        m_curved.angle_tolerance(0.2);
                    }
                    ras.reset();
                    ras.filling_rule(fill_non_zero);
                    ras.add_path(m_curved_stroked_trans, attr.index);
                    if(attr.stroke_url[0] != 0)
                    {
                        if (m_paint_by_label) 
                            agg::render_scanlines_bin_solid(ras, sl, rb, m_label_color);
                        else
                          render_gradient(ras,sl,rb,attr.stroke_url,attr.opacity);
                    }
                    else
                    {
                        if (m_paint_by_label) {
                            agg::render_scanlines_bin_solid(ras, sl, rb, m_label_color);
                        }
                        else {
                            if (color_multiplier != 1.0)
                                color = modify_color(attr.stroke_color, color_multiplier, color_offset);
                            else
                                color = attr.stroke_color;
                            color = attr.stroke_color;
                            color.opacity(color.opacity() * opacity * attr.opacity);
                            ren.color(color);
                            agg::render_scanlines(ras, sl, ren);
                        }
                    }
                }
            }
        }

        void transform(trans_affine transform)
        {
            m_user_transform = transform * m_user_transform;
        }

        void pre_transform(trans_affine transform)
        {
            m_user_transform = m_user_transform * transform;
        }

        void reset_transform()
        {
            m_user_transform = trans_affine();
        }

        bool empty() const
        {
            return m_attr_storage.size()==0;
        }

        trans_affine get_transform() const
        {
            return m_user_transform;
        }

        void start_gradient(bool radial = false);
        void end_gradient();
        gradient* current_gradient() const { return m_cur_gradient; }
        double width_in_mm() const { return m_width_in_mm; }
        double height_in_mm() const { return m_height_in_mm; }
        double width_in_pt() const { return m_width_in_mm*SVG_MM_TO_POINT; }
        double height_in_pt() const { return m_height_in_mm*SVG_MM_TO_POINT; }
        void set_width_in_mm(double width_in_mm) { m_width_in_mm = width_in_mm; }
        void set_height_in_mm(double height_in_mm) { m_height_in_mm = height_in_mm; }
        void set_label_color(rgba label_color)
        {
          m_paint_by_label = true;
          m_label_color = label_color;
        }
        void set_paint_by_label(bool paint_by_label)
        {
          m_paint_by_label = paint_by_label;
        }
        
    private:
        void	add_gradient(gradient* gradient);
        gradient*	gradient_at(int32 index) const;
        gradient*	find_gradient(const char* name) const;
        path_attributes& cur_attr();

        path_storage   m_storage;
        attr_storage   m_attr_storage;
        attr_storage   m_attr_stack;
        trans_affine   m_transform;
        trans_affine   m_user_transform;
        double         m_width_in_mm, m_height_in_mm;
        std::vector<gradient*> m_gradients;
        gradient*	m_cur_gradient;

        curved                       m_curved;
        curved_count                 m_curved_count;

        curved_stroked               m_curved_stroked;
        curved_stroked_trans         m_curved_stroked_trans;

        curved_trans                 m_curved_trans;
        curved_trans_contour         m_curved_trans_contour;
        bool m_paint_by_label;
        rgba m_label_color;
    };

}
}

#endif
