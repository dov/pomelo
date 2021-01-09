//======================================================================
//  GivPainterCairo.cc - A painter for cairo
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Thu Apr 24 18:04:14 2008
//----------------------------------------------------------------------

#include "GivPainterCairo.h"
#include "glib.h"
#include "math.h"

#define MY_PI 3.141592653589793

static
void draw_arrows(cairo_t *cr,
                 double stroke_width,
                 bool do_start_arrow,
                 bool do_end_arrow,
                 double arrow_d1,
                 double arrow_d2,
                 double arrow_d3,
                 double arrow_d4,
                 double arrow_d5);
static void
draw_arrow(cairo_t *cr,
           double x,
           double y,
           double dx,
           double dy,
           double arrow_d1,
           double arrow_d2,
           double arrow_d3,
           double arrow_d4,
           double arrow_d5);

class GivPainterCairo::Priv {
public:
    Priv() {}
    cairo_t *cr;
    gboolean do_paint_by_index;
    double last_x;
    double last_y;
    int set_idx;
    bool need_stroke;
    bool need_fill;
    bool do_antialiased;
    bool swap_blue_red;
    bool color_transparent;
    PangoFontDescription *font_description;
    PangoLayout *layout;
    double arrow_d1;
    double arrow_d2;
    double arrow_d3;
    double arrow_d4;
    double arrow_d5;
    bool do_start_arrow;
    bool do_end_arrow;
    double stroke_width;
};

GivPainterCairo::GivPainterCairo(cairo_t *cr,
                                 bool do_antialiased)
{
    d = new Priv();
    d->last_x = -1;
    d->last_y = -1;
    d->set_idx = -1;
    d->need_stroke = false;
    d->need_fill = false;
    d->do_paint_by_index = false;
    d->swap_blue_red = false;
    d->color_transparent = false;
    d->font_description = NULL;
    d->layout = NULL;
    d->arrow_d1=0;
    d->arrow_d2=3;
    d->arrow_d3=2;
    d->arrow_d4=2;
    d->arrow_d5=0.5;
    d->do_start_arrow = false;
    d->do_end_arrow = false;
    d->stroke_width = 1;
    set_cairo(cr, do_antialiased);
}

GivPainterCairo::~GivPainterCairo()
{
    if (d->font_description)
        pango_font_description_free(d->font_description);
    if (d->layout) 
        g_object_unref(G_OBJECT(d->layout));
    delete d;
}

void
GivPainterCairo::set_cairo(cairo_t *cr,
                           bool do_antialiased)
{
    d->cr = cr;
    if (!cr)
        return;

    // Setup defaults
    if (d->layout)
        g_object_unref(G_OBJECT(d->layout));

    d->layout = pango_cairo_create_layout (cr);
    cairo_set_line_cap(d->cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(d->cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_antialias(d->cr,
                        (do_antialiased
                         ? CAIRO_ANTIALIAS_DEFAULT : CAIRO_ANTIALIAS_NONE));
}
    
void
GivPainterCairo::set_swap_blue_red(bool whether)
{
    d->swap_blue_red = whether;
}

void
GivPainterCairo::set_set_idx(int set_idx)
{
    d->set_idx = set_idx;
}

void
GivPainterCairo::set_color(double red, double green, double blue, double alpha)
{
    if (d->do_paint_by_index) {
        double rr,gg,bb;
        label_to_color(d->set_idx,
                       // output
                       rr,gg,bb);
        cairo_set_source_rgba (d->cr, rr,gg,bb,1);
        d->color_transparent = false;
    }
    else if (red < 0 || green < 0 || blue < 0) 
        d->color_transparent = true;
    else {
        d->color_transparent = false;
        if (d->swap_blue_red) 
            // Switch colors for red and blue because of the difference
            // between the PixBuf and the cairo surface order
            cairo_set_source_rgba (d->cr, blue,green,red,alpha);
        else 
            cairo_set_source_rgba (d->cr, red,green,blue,alpha);
    }
}

int
GivPainterCairo::set_line_width(double line_width)
{
    if (d->do_paint_by_index && line_width < 3)
        line_width = 3;
    cairo_set_line_width (d->cr, line_width);
    d->stroke_width = line_width;
    
    return 0;
}

int
GivPainterCairo::add_mark(GivMarkType mark_type,
                          double mark_size_x, double mark_size_y,
                          double x, double y)
{
    double rx=mark_size_x/2, ry=mark_size_y/2; // Mark size
    if (mark_type == MARK_TYPE_CIRCLE) {
        cairo_move_to(d->cr, x+rx,y);
        cairo_arc(d->cr,
                  x, y,
                  rx, 0.0, 2*G_PI);
        d->need_stroke = 1;
    }
    else if (mark_type == MARK_TYPE_FCIRCLE) {
        cairo_arc(d->cr,
                  x, y,
                  rx, 0.0, 2*G_PI);
        d->need_fill = 1;
    }
    else if (mark_type == MARK_TYPE_SQUARE) {
        cairo_move_to(d->cr, x-rx,y-ry);
        cairo_rectangle(d->cr, x-rx,y-ry,2*rx,2*ry);
        d->need_stroke = 1;
    }
    else if (mark_type == MARK_TYPE_FSQUARE) {
        cairo_rectangle(d->cr, x-rx,y-ry,2*rx,2*ry);
        d->need_fill = 1;
    }

    return 0;
}

int GivPainterCairo::add_ellipse(double x, double y,
                                 double sizex, double sizey,
                                 double angle)
{
    // TBD - Test if this is ok
    printf("add_ellipse cairo\n");
    cairo_save(d->cr);
    cairo_translate(d->cr, -x, -y);
    cairo_rotate(d->cr, angle);
    cairo_translate(d->cr, x, y);
    cairo_scale(d->cr, 1.0, sizey/sizex);
    cairo_arc(d->cr, x, y, sizex, 0, 2*MY_PI);
    cairo_fill(d->cr);
    cairo_restore(d->cr);
    d->need_fill = 1;

    return 0;
}

int 
GivPainterCairo::add_text(const char *text,
                          double x, double y,
                          int text_align,
                          bool do_pango_markup)
{
    PangoRectangle logical_rect;
    // Translate the 1-9 square to alignment
    double h_align = 1.0*((text_align - 1) % 3) / 2.0;
    double v_align = 1.0-(1.0*((text_align - 1) / 3) / 2.0);
    if (do_pango_markup)
        pango_layout_set_markup(d->layout, text, -1);
    else
        pango_layout_set_text (d->layout, text, -1);
    pango_layout_get_extents(d->layout,
                             NULL,
                             &logical_rect);
    cairo_move_to(d->cr,
                  x-h_align/PANGO_SCALE*logical_rect.width,
                  y-v_align/PANGO_SCALE*logical_rect.height);
    pango_cairo_show_layout(d->cr, d->layout);
    
#if 0
    cairo_set_font_size(d->cr, 30);
    cairo_move_to(d->cr, x,y);
    cairo_show_text(d->cr, text);
#endif
    d->need_fill = true;
    return 0;
}

int
GivPainterCairo::add_line_segment(double x0, double y0,
                                  double x1, double y1,
                                  bool do_polygon)
{
    if (d->last_x != x0 || d->last_y != y0) 
        cairo_move_to (d->cr,  x0, y0 );
    cairo_line_to (d->cr,  x1, y1 );
    d->last_x = x1;
    d->last_y = y1;
    d->need_stroke = true;
    
    return 0;
}

void
GivPainterCairo::draw_marks()
{
    if (d->need_stroke)
        cairo_stroke (d->cr);
    if (d->need_fill)
        cairo_fill (d->cr);
    d->last_x = -1;
    d->last_y = -1;
    d->need_stroke = false;
    d->need_fill = false;
}

void
GivPainterCairo::fill()
{
    if (!d->color_transparent)
        cairo_fill (d->cr);
    else
        cairo_new_path(d->cr);
        
    d->last_x = -1;
    d->last_y = -1;
    d->need_stroke = false;
    d->need_fill = false;
}

void
GivPainterCairo::stroke()
{
    if (!d->color_transparent) {
        if (d->do_start_arrow
            || d->do_end_arrow)
            draw_arrows(d->cr,
                        d->stroke_width,
                        d->do_start_arrow,
                        d->do_end_arrow,
                        d->arrow_d1,
                        d->arrow_d2,
                        d->arrow_d3,
                        d->arrow_d4,
                        d->arrow_d5
                    );
        cairo_stroke(d->cr);
    }
    else
        cairo_new_path(d->cr);
    d->last_x = -1;
    d->last_y = -1;
    d->need_stroke = false;
    d->need_fill = false;
}

void
GivPainterCairo::close_path()
{
    cairo_close_path(d->cr);
}

void
GivPainterCairo::set_do_paint_by_index(bool do_paint_by_index)
{
    d->do_paint_by_index = do_paint_by_index;
}

int
GivPainterCairo::set_text_size(double text_size)
{
    if (d->font_description) {
        pango_font_description_set_size(d->font_description,
                                        text_size*PANGO_SCALE);
        pango_layout_set_font_description(d->layout,
                                          d->font_description);
    }
    return 0;
}

int
GivPainterCairo::set_font(const char* font_name)
{
    if (d->font_description)
        pango_font_description_free(d->font_description);
    d->font_description = pango_font_description_from_string(font_name);
    pango_layout_set_font_description(d->layout,
                                      d->font_description);

    return 0;
}

void
GivPainterCairo::set_dashes(int num_dashes,
                            double* dashes)
{
    cairo_set_dash(d->cr,
                   dashes,
                   num_dashes,
                   0);
}

void
GivPainterCairo::set_arrow(bool do_start_arrow,
                           bool do_end_arrow,
                           double arrow_d1,
                           double arrow_d2,
                           double arrow_d3,
                           double arrow_d4,
                           double arrow_d5
                           )
{
    // This currently just stores the info
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

    d->do_start_arrow = do_start_arrow;
    d->do_end_arrow = do_end_arrow;
}

void GivPainterCairo::render_svg_path(agg::svg::path_renderer *svg,
                                      double mx, double my,
                                      double scalex, double scaley)
{
  // TBD - Use svgpp or resvg!
}

void
GivPainterCairo::label_to_color(int label,
                                // output
                                double& rr,
                                double& gg,
                                double& bb
                                )
{
    rr = 1.0/255*((label+1) % 256);
    gg = 1.0/255*(((label+1) >> 8) % 256);
    bb = 1.0/255*(((label+1) >> 16) % 256);
}

static
void draw_arrows(cairo_t *cr,
                 double stroke_width,
                 bool do_start_arrow,
                 bool do_end_arrow,
                 double arrow_d1,
                 double arrow_d2,
                 double arrow_d3,
                 double arrow_d4,
                 double arrow_d5)
{
    // Strategy:
    //   * Extract current path
    //   * Erase the current path
    //   * Modify the path to make it shorter so that the arrow
    //     will not protrude.
    //   * Fill paths for the arrows heads.
    //   * Draw the shortened path.

    // Extract the current path
    cairo_path_t *path= cairo_copy_path(cr);

    // If the path is closed then just get out!
    int n = path->num_data;
    if (path->data[n-1].header.type == CAIRO_PATH_CLOSE_PATH) {
        cairo_path_destroy(path);
        return;
    }

    // Erase old path
    cairo_new_path(cr);

    arrow_d1 *= stroke_width;
    arrow_d2 *= stroke_width;
    arrow_d3 *= stroke_width;
    arrow_d4 *= stroke_width;
    arrow_d5 *= stroke_width;

    // Shorten paths due to arrows
    double short_dist = 0*arrow_d1; // TBD
    cairo_path_data_t *pdata = NULL;
    for (int i=0; i<n; i+= path->data[i].header.length) {
        cairo_path_data_t *data = &path->data[i];
        cairo_path_data_t *ndata = &path->data[i+path->data[i].header.length];

        // start paths
        if (do_start_arrow
            && data->header.type == CAIRO_PATH_MOVE_TO) {
            double dx = ndata[1].point.x-data[1].point.x;
            double dy = ndata[1].point.y-data[1].point.y;
            double dz = sqrt(dx*dx+dy*dy);

            // draw arrow and shorten path
            if (dz > 0) {
                draw_arrow(cr, data[1].point.x, data[1].point.y, dx, dy,
                           arrow_d1, arrow_d2, arrow_d3, arrow_d4, arrow_d5);
                
                data[1].point.x += dx * short_dist/dz;
                data[1].point.y += dy * short_dist/dz;
            }
        }

        // end paths
#if 0
        printf("do_end_arrow pdata = %d %d i+...=%d n-1=%d\n", do_end_arrow, pdata, i+data->header.length,n);
#endif
        if (do_end_arrow
            && pdata
            && (i+data->header.length >= n
                || ndata->header.type == CAIRO_PATH_MOVE_TO)) {
            double dx = data[1].point.x-pdata[1].point.x;
            double dy = data[1].point.y-pdata[1].point.y;
            double dz = sqrt(dx*dx+dy*dy);
            
#if 0
            printf("dz = %f\n", dz);
#endif

            // draw arrow and shorten path
            if (dz > 0) {
                draw_arrow(cr, data[1].point.x, data[1].point.y, -dx, -dy,
                           arrow_d1, arrow_d2, arrow_d3, arrow_d4, arrow_d5);

                data[1].point.x -= dx * short_dist/dz;
                data[1].point.y -= dy * short_dist/dz;
            }
        }
        pdata = data;
    }

    cairo_append_path(cr, path);
}                 

static void
draw_arrow(cairo_t *cr,
           double x,
           double y,
           double dx,
           double dy,
           double arrow_d1,
           double arrow_d2,
           double arrow_d3,
           double arrow_d4,
           double arrow_d5)
{
    double angle = atan2(dy,dx);
    cairo_save(cr);
    cairo_translate(cr, x,y);
    cairo_rotate(cr, angle);
    cairo_move_to(cr, -arrow_d1,-arrow_d5);
    cairo_line_to(cr, arrow_d2+arrow_d4, -arrow_d3);
    cairo_line_to(cr, arrow_d2, 0);
    cairo_line_to(cr, arrow_d2+arrow_d4, arrow_d3);
    cairo_line_to(cr, -arrow_d1, arrow_d5);
    cairo_close_path(cr);
    cairo_fill(cr);
    cairo_restore(cr);
}

