//======================================================================
//  GivPainterCairoPixbuf.cc - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Wed Apr 30 21:18:53 2008
//----------------------------------------------------------------------

#include "GivPainterCairoPixbuf.h"

class GivPainterCairoPixbuf::Priv
{
 public:
    cairo_surface_t *surface;
    cairo_t *cairo;
};

GivPainterCairoPixbuf::GivPainterCairoPixbuf(GdkPixbuf *pixbuf,
                                             bool do_antialiased)
{
    d = new Priv;
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    d->surface = cairo_image_surface_create_for_data
        (gdk_pixbuf_get_pixels(pixbuf),
         CAIRO_FORMAT_RGB24,
         width,
         height,
         gdk_pixbuf_get_rowstride(pixbuf));
    d->cairo = cairo_create (d->surface);

    ((GivPainterCairo*)this)->set_cairo(d->cairo,
                                        do_antialiased);
    ((GivPainterCairo*)this)->set_swap_blue_red(true);
}

GivPainterCairoPixbuf::~GivPainterCairoPixbuf()
{
    cairo_surface_destroy(d->surface);
    cairo_destroy(d->cairo);
    delete d;
}
