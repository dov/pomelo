//======================================================================
//  GivPainterAgg.cpp - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Tue Nov  6 22:18:10 2007
//----------------------------------------------------------------------

#include <math.h>
#include <string>
#include "GivPainterAgg.h"
#include "agg/agg_rasterizer_scanline_aa.h"
#include "agg/agg_ellipse.h"
#include "agg/agg_arc.h"
#include "agg/agg_conv_curve.h"
#include "agg/agg_conv_dash.h"
#include "agg/agg_scanline_p.h"
#include "agg/agg_renderer_scanline.h"
#include "agg/agg_path_storage.h"
#include "agg/agg_pixfmt_rgb.h"
#include "agg/agg_pixfmt_rgba.h"
#include "agg/agg_vcgen_stroke.h"
#include "agg/agg_conv_stroke.h"
#include "agg/agg_conv_marker.h"
#include "agg/agg_vcgen_markers_term.h"
#include "agg_conv_clipper.h"
#include "giv_agg_arrowhead.h"
#include "cairo.h"
#include "math.h"

using namespace std;

class GivPainterAggPriv {
public:
    GivPainterAggPriv(GdkPixbuf *pixbuf,
                      bool do_antialiased);
    ~GivPainterAggPriv();
    int width;
    int height;
    double stroke_width;
    agg::rendering_buffer rbuf;
    int set_idx;
    double red, green, blue, alpha;
    agg::rasterizer_scanline_aa<> pf;
    agg::path_storage path;
    agg::path_storage bbox;
    agg::scanline_p8 sl;
    agg::vcgen_stroke stroke;
    agg::pixfmt_rgba32 pixf;
    agg::renderer_base<agg::pixfmt_rgba32> rbase;
    agg::conv_curve<agg::path_storage> curve;
    agg::conv_stroke<agg::conv_curve<agg::path_storage>,
                     agg::vcgen_markers_term > cstroke;
    agg::conv_dash<
        agg::conv_curve<agg::path_storage>, agg::vcgen_markers_term > dash;
    agg::conv_stroke<
        agg::conv_dash<
        agg::conv_curve<
            agg::path_storage>, agg::vcgen_markers_term > > stroke_dash;
    givagg::arrowhead arrowhead;
    bool do_antialiased;
    bool do_paint_by_index;
    bool do_dash;
    bool do_start_arrow;
    bool do_end_arrow;
    double arrow_d1, arrow_d2, arrow_d3, arrow_d4, arrow_d5;
    string font;
    double old_x, old_y;
    PangoFontDescription *font_description;
    PangoLayout *layout;
    agg::svg::path_renderer  *svg_mark = nullptr;

    // used for writing text
    cairo_t *cr;
    cairo_surface_t *surface;

};

GivPainterAggPriv::GivPainterAggPriv(GdkPixbuf *pixbuf,
                                     bool _do_anti_aliased)
    : width(gdk_pixbuf_get_width(pixbuf)),
      height(gdk_pixbuf_get_height(pixbuf)),
      rbuf(gdk_pixbuf_get_pixels(pixbuf), 
           width, 
           height,
           gdk_pixbuf_get_rowstride(pixbuf)),
      set_idx(0),
      red(0),
      green(0),
      blue(0),
      alpha(1.0),
      pixf(rbuf),
      rbase(pixf),
      curve(path),
      cstroke(curve),
      dash(curve),
      stroke_dash(dash),
      do_antialiased(_do_anti_aliased),
      do_paint_by_index(false),
      do_dash(false),
      do_start_arrow(false),
      do_end_arrow(false),
      arrow_d1(0),
      arrow_d2(3),
      arrow_d3(2),
      arrow_d4(2),
      arrow_d5(1),
      old_x(-1),
      old_y(-1),
      font_description(NULL),
      layout(NULL)
{
    surface = cairo_image_surface_create_for_data
        (gdk_pixbuf_get_pixels(pixbuf),
         CAIRO_FORMAT_RGB24,
         width,
         height,
         gdk_pixbuf_get_rowstride(pixbuf));
    cr = cairo_create (surface);
    cairo_set_antialias(cr,
                        (do_antialiased
                         ? CAIRO_ANTIALIAS_DEFAULT : CAIRO_ANTIALIAS_NONE));
    stroke.line_join(agg::miter_join_round);
    stroke.line_cap(agg::round_cap);
    cstroke.line_join(agg::miter_join_round);
    cstroke.line_cap(agg::round_cap);
    stroke_dash.line_join(agg::miter_join_round);
    stroke_dash.line_cap(agg::round_cap);
    layout = pango_cairo_create_layout (cr);

    // Create a bounding box that will be used for clipping
    bbox.move_to(0, 0);
    bbox.line_to(0, height);
    bbox.line_to(width,height);
    bbox.line_to(width,0);
    bbox.close_polygon();
}

GivPainterAggPriv::~GivPainterAggPriv()
{
    if (font_description)
        pango_font_description_free(font_description);
    if (layout)
        g_object_unref(G_OBJECT(layout));
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

GivPainterAgg::GivPainterAgg(GdkPixbuf *pixbuf,
                             bool do_antialiased)
{
    d = new GivPainterAggPriv(pixbuf, do_antialiased);
    set_font("Sans 15");
}

GivPainterAgg::~GivPainterAgg()
{
    delete d;
}


void GivPainterAgg::set_set_idx(int set_idx)
{
    d->set_idx = set_idx;
}

void GivPainterAgg::set_color(double red,
                              double green,
                              double blue,
                              double alpha)
{
    d->red = red;
    d->green = green;
    d->blue = blue;
    d->alpha = alpha;
}

int GivPainterAgg::set_line_width(double line_width)
{
    if (d->do_paint_by_index && line_width < 3)
        line_width = 3;
    d->cstroke.width(line_width);
    d->stroke_dash.width(line_width);
    if (d->do_start_arrow || d->do_end_arrow)
        set_arrow(d->do_start_arrow,
                  d->do_end_arrow);
    return 0;
}

void GivPainterAgg::add_svg_mark(double x, double y, double sx, double sy) 
{
  render_svg_path(d->svg_mark,
                  x,y,
                  sx,sy);
}

// This is really too high level to be in the painter class, which
// should restrict itself to drawing circles, and polygons etc.
// In any case, it should be optimized!
int GivPainterAgg::add_mark(GivMarkType mark_type,
                            double mark_size_x, double mark_size_y,
                            double x, double y)
{
    double rx=mark_size_x/2, ry=mark_size_y/2; // Mark size
    if (mark_type == MARK_TYPE_CIRCLE) {
        agg::ellipse ell;
        ell.init(x,y,rx,ry);
        agg::conv_stroke<agg::ellipse> ell_stroke(ell);
        ell_stroke.width(d->cstroke.width());

        d->pf.add_path(ell_stroke);
    }
    else if (mark_type == MARK_TYPE_FCIRCLE) {
        agg::ellipse ell;
        ell.init(x,y,rx,ry);
        d->pf.add_path(ell);
    }
    else if (mark_type == MARK_TYPE_SQUARE) {
        d->path.remove_all();
        d->path.move_to(x-rx, y-ry);
        d->path.line_to(x+rx, y-ry);
        d->path.line_to(x+rx, y+ry);
        d->path.line_to(x-rx, y+ry);
        d->path.close_polygon();
        d->pf.add_path(d->cstroke);
        d->path.remove_all();
    }
    else if (mark_type == MARK_TYPE_FSQUARE) {
        d->pf.move_to_d(x-rx, y-ry);
        d->pf.line_to_d(x+rx, y-ry);
        d->pf.line_to_d(x+rx, y+ry);
        d->pf.line_to_d(x-rx, y+ry);
        d->pf.line_to_d(x-rx, y-ry);
    }

    return 0;
}

int GivPainterAgg::add_ellipse(double x, double y,
                               double sizex, double sizey,
                               double angle)
{
    agg::ellipse ell;
    agg::trans_affine mtx;
    mtx *= agg::trans_affine_translation(-x,-y);
    mtx *= agg::trans_affine_rotation(angle);
    mtx *= agg::trans_affine_translation(x,y);
    
    ell.init(x,y,sizex/2.0,sizey/2.0);
    agg::conv_transform<agg::ellipse, agg::trans_affine> trans(ell, mtx);
    d->pf.add_path(trans);

    return 0;
}

int
GivPainterAgg::add_text(const char *text,
                        double x, double y,
                        int text_align,
                        bool do_pango_markup)
{
    // ignore zero length texts
    if (text==NULL || strlen(text)==0)
        return 0;

    if (d->do_paint_by_index) {
        return 0; // Don't paint text labels...
#if 0
        double rr,gg,bb;
        label_to_color(d->set_idx,
                       // output and swap bb and rr because of cairo
                       bb,gg,rr);
        cairo_set_source_rgba (d->cr, bb,gg,rr,1);
#endif
    }
    else {
        // Switch colors for red and blue because of the difference
        // between the PixBuf and the cairo surface order
        cairo_set_source_rgba (d->cr, d->blue,d->green,d->red,d->alpha);
    }

    /* Create a PangoLayout, set the font and text */
    PangoRectangle logical_rect;

    // Translate the 1-9 square to alignment
    double h_align = 1.0*((text_align - 1) % 3) / 2.0;
    double v_align = 1.0-(1.0*((text_align - 1) / 3) / 2.0);
    if (do_pango_markup)
        pango_layout_set_markup(d->layout, text, -1);
    else
        pango_layout_set_text (d->layout, text, -1);
    pango_layout_set_alignment(d->layout, PangoAlignment((text_align-1)%3));

    pango_layout_get_extents(d->layout,
                             NULL,
                             &logical_rect);
    cairo_move_to(d->cr,
                  x-h_align/PANGO_SCALE*logical_rect.width,
                  y-v_align/PANGO_SCALE*logical_rect.height);
    pango_cairo_show_layout(d->cr, d->layout);


#if 0
    cairo_set_font_size(d->cr, 15);
    cairo_move_to(d->cr, x,y);
    cairo_show_text(d->cr, text);
    cairo_fill(d->cr);
#endif
    
    return 0;
}

int GivPainterAgg::add_line_segment(double x0, double y0,
                                    double x1, double y1,
                                    bool /*do_polygon*/)
{
    if (d->old_x != x0
        || d->old_y != y0)
        d->path.move_to(x0, y0);
    d->path.line_to(x1, y1);
    d->old_x = x1;
    d->old_y = y1;

    return 0;
}

void GivPainterAgg::fill()
{
    agg::rgba color;
    if (d->do_paint_by_index) {
        double rr, gg, bb;
        label_to_color(d->set_idx,
                       // output
                       rr, gg, bb);
        color = agg::rgba(rr,gg,bb,1);
    }
    else {
        if (d->red < 0
            || d->green < 0
            || d->blue < 0) {
            d->pf.reset();
            d->path.remove_all();
            
            return;
        }
        color = agg::rgba(d->red,d->green,d->blue,d->alpha);
    }

    if (d->do_paint_by_index) {
        double rr, gg, bb;
        label_to_color(d->set_idx,
                       // output
                       rr, gg, bb);
        color = agg::rgba(rr,gg,bb,1);
    }

    typedef agg::conv_clipper<agg::path_storage, agg::path_storage> poly_clipper;
    poly_clipper clipped(d->path,
                         d->bbox,
                         agg::clipper_and,
                         agg::clipper_non_zero,
                         agg::clipper_non_zero);
    
    d->pf.add_path(clipped);
    if (d->do_antialiased)
        agg::render_scanlines_aa_solid(d->pf, d->sl, d->rbase, color);
    else
        agg::render_scanlines_bin_solid(d->pf, d->sl, d->rbase, color);
    d->pf.reset();
    d->path.remove_all();
    d->old_x = d->old_y = NAN;
}

void GivPainterAgg::close_path()
{
    d->path.close_polygon(); 
}

void GivPainterAgg::draw_marks()
{
    fill();
}

void GivPainterAgg::stroke()
{
    agg::rgba color;

    if (d->do_paint_by_index) {
        double rr, gg, bb;
        label_to_color(d->set_idx,
                       // output
                       rr, gg, bb);
        color = agg::rgba(rr,gg,bb,1.0);
    }
    else {
        if (d->red < 0
            || d->green < 0
            || d->blue < 0) {
            d->pf.reset();
            d->path.remove_all();
            
            return;
        }
        color = agg::rgba(d->red,d->green,d->blue,d->alpha);
    }
    
    if (d->do_dash) {
        d->pf.add_path(d->stroke_dash);
    }
    else {
        d->pf.add_path(d->cstroke);
    }
    if (d->do_start_arrow||d->do_end_arrow) {
        agg::vcgen_markers_term *m;

        if (d->do_dash)
            m = &d->dash.markers();
        else
            m = &d->cstroke.markers();
        agg::conv_marker<agg::vcgen_markers_term, givagg::arrowhead> arrow(*m, d->arrowhead);
        d->pf.add_path(arrow);
    }
    if (d->do_antialiased)
        agg::render_scanlines_aa_solid(d->pf, d->sl, d->rbase, color);
    else
        agg::render_scanlines_bin_solid(d->pf, d->sl, d->rbase, color);
    d->pf.reset();
    d->path.remove_all();
    d->old_x = NAN;
    d->old_y = NAN;
}

void GivPainterAgg::set_do_paint_by_index(bool do_paint_by_index)
{
    d->do_paint_by_index = do_paint_by_index;
}
  
int
GivPainterAgg::set_text_size(double text_size)
{
    if (d->font_description) {
        pango_font_description_set_size(d->font_description,
                                        text_size*PANGO_SCALE);
        pango_layout_set_font_description(d->layout,
                                          d->font_description);
    }

    return 0;
}

int GivPainterAgg::set_font(const char* font)
{
    if (d->font_description)
        pango_font_description_free(d->font_description);
    d->font_description = pango_font_description_from_string(font);
    pango_layout_set_font_description(d->layout,
                                      d->font_description);

    return 0;
}

void
GivPainterAgg::set_dashes(int num_dashes,
                          double* dashes)
{
    d->do_dash = (num_dashes > 0);
    d->dash.remove_all_dashes();
    for (int i=0; i<num_dashes/2; i++) 
        d->dash.add_dash(dashes[2*i], dashes[2*i+1]);
}

void
GivPainterAgg::set_arrow(bool do_start_arrow,
                         bool do_end_arrow,
                         double arrow_d1,
                         double arrow_d2,
                         double arrow_d3,
                         double arrow_d4,
                         double arrow_d5)
{
    double width = d->cstroke.width();
    double k = width;
    if (arrow_d1<0)
        arrow_d1 = d->arrow_d1;
    if (arrow_d2<0)
        arrow_d2 = d->arrow_d2;
    if (arrow_d3<0)
        arrow_d3 = d->arrow_d3;
    if (arrow_d4<0)
        arrow_d4= d->arrow_d4;
    if (arrow_d5<0)
        arrow_d5= d->arrow_d5;
    d->arrowhead.set_arrow(arrow_d1*k,arrow_d2*k,arrow_d3*k,arrow_d4*k, width/2);
    if (do_end_arrow) 
        d->arrowhead.head();
    else
        d->arrowhead.no_head();
    if (do_start_arrow) 
        d->arrowhead.tail();
    else
        d->arrowhead.no_tail();
    d->do_start_arrow = do_start_arrow;
    d->do_end_arrow = do_end_arrow;
}

void
GivPainterAgg::label_to_color(int label,
                              // output
                              double& rr,
                              double& gg,
                              double& bb
                              )
{
    bb = 1.0/255*((label+1) % 256);
    gg = 1.0/255*(((label+1) >> 8) % 256);
    rr = 1.0/255*(((label+1) >> 16) % 256);
}

void GivPainterAgg::render_svg_path(agg::svg::path_renderer *svg,
                                    double mx, double my,
                                    double scalex, double scaley)
{
  agg::trans_affine AggTransform(scalex,0,0,scaley,mx,my); 

  // Render the svg in the buffer.
  if (d->do_paint_by_index)
    {
      agg::rgba label_color;
      double rr, gg, bb;
      label_to_color(d->set_idx,
                     // output
                     rr, gg, bb);
      label_color = agg::rgba(rr,gg,bb,1);
      svg->set_label_color(label_color);
    }
  else
    svg->set_paint_by_label(false);
  svg->render(d->pf, d->sl, d->rbase,
              AggTransform,
              d->rbase.clip_box()
              );
}

int GivPainterAgg::set_svg_mark(agg::svg::path_renderer *svg_mark)
{
  d->svg_mark = svg_mark;
  return 0;
}
