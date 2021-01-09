//======================================================================
//  GivPainterCairo.h - A painter for cairo. May be used for
//                      creating svg.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Thu Apr 24 18:02:52 2008
//----------------------------------------------------------------------
#ifndef GIVPAINTERCAIRO_H
#define GIVPAINTERCAIRO_H

#include "GivPainter.h"

class GivPainterCairo : public GivPainter {
 public:
    GivPainterCairo(cairo_t *cairo = NULL,
                    bool do_antialiased = false);
    virtual ~GivPainterCairo();

    void set_cairo(cairo_t *cairo,
                   bool do_antialiased);
    void set_swap_blue_red(bool whether);
    void set_set_idx(int set_idx) override;
    void set_color(double red, double green, double blue, double alpha) override;
    int set_line_width(double line_width) override;

    int add_mark(GivMarkType mark_type,
                         double mark_size_x, double mark_size_y,
                         double x, double y) override;
    int add_ellipse(double x, double y,
                            double sizex, double sizey,
                            double angle) override;
    int add_text(const char *text,
                         double x, double y,
                         int text_align,
                         bool do_pango_markup) override;
    int add_line_segment(double x0, double y0,
                                 double x1, double y1,
                                 bool do_polygon=false) override;
    void fill() override;
    void stroke() override;
    void close_path() override;
    void draw_marks() override;
    void set_do_paint_by_index(bool do_paint_by_index) override;
    int set_text_size(double text_size) override;
    int set_font(const char* font_name) override;
    void set_dashes(int num_dashes,
                    double* dashes) override;
    void set_arrow(bool do_start_arrow,
                   bool do_end_arrow,
                   double arrow_d1=-1,
                   double arrow_d2=-1,
                   double arrow_d3=-1,
                   double arrow_d4=-1,
                   double arrow_d5=-1
                   ) override;
    
    void render_svg_path(agg::svg::path_renderer *svg,
                         double mx, double my,
                         double scalex, double scaley) override;

    // Not supported yet
    int set_svg_mark(agg::svg::path_renderer *svgmark) override {return 0;}
    void add_svg_mark(double x, double y, double sx, double sy) override {}

    static void label_to_color(int label,
                               // output
                               double& rr,
                               double& gg,
                               double& bb);

 private:
    class Priv;
    Priv *d;
};

#endif /* GIVPAINTERCAIRO */
