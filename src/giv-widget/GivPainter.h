//======================================================================
//  GivPainter.h - A virtual base class for a pointer class
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Tue Nov  6 22:01:52 2007
//----------------------------------------------------------------------

#ifndef GIV_PAINTER_H
#define GIV_PAINTER_H

#include "giv-data.h"

class GivPainter {
 public:
    GivPainter() {}
    virtual ~GivPainter() {}

    virtual void set_set_idx(int set_idx) {}
    virtual void set_color(double red,
                           double green,
                           double blue,
                           double alpha=1.0) = 0;
    void set_giv_color(GivColor giv_color)
    {
        static const double s = 1.0/0xffff;
        set_color(giv_color.red * s,
                  giv_color.green * s,
                  giv_color.blue * s,
                  giv_color.alpha * s);
    }
    virtual int set_line_width(double line_width) = 0;
    virtual int set_svg_mark(agg::svg::path_renderer *svgmark) = 0;
    virtual int set_text_size(double text_size) = 0;
    virtual int set_font(const char* font) = 0;
    virtual int add_mark(GivMarkType mark_type,
                         double mark_size_x, double mark_size_y,
                         double x, double y) = 0;
    virtual int add_ellipse(double x, double y,
                            double sizex, double sizey,
                            double angle) = 0;
    virtual int add_line_segment(double x0, double y0,
                                 double x1, double y1,
                                 bool do_polygon = 0) = 0;
    virtual int add_text(const char *text,
                         double x, double y,
                         int text_align,
                         bool do_pango_markup) = 0;
    virtual void draw_marks() = 0;
    virtual void fill() = 0;
    virtual void stroke() = 0;
    virtual void close_path() = 0;
    virtual void set_do_paint_by_index(bool do_paint_by_index) = 0;

    virtual void set_dashes(int num_dashes,
                            double* dashes) = 0;
    virtual void set_arrow(bool do_start_arrow,
                           bool do_end_arrow,
                           double arrow_d1=-1,
                           double arrow_d2=-1,
                           double arrow_d3=-1,
                           double arrow_d4=-1,
                           double arrow_d5=-1
                           )=0;
    virtual void render_svg_path(agg::svg::path_renderer *svg,
                                 double mx, double my,
                                 double scalex, double scaley) = 0;
    virtual void add_svg_mark(double x, double y, double sx, double sy) = 0;
};

#endif
