//======================================================================
//  GivPainterCairoPixbuf.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Wed Apr 30 21:15:34 2008
//----------------------------------------------------------------------
#ifndef GIVPAINTERCAIROPIXBUF_H
#define GIVPAINTERCAIROPIXBUF_H

#include "GivPainterCairo.h"

class GivPainterCairoPixbuf : public GivPainterCairo {
 public:
    GivPainterCairoPixbuf(GdkPixbuf *pixbuf,
                          bool do_antialiased);
    virtual ~GivPainterCairoPixbuf();

 private:
    class Priv;
    Priv *d;
};

#endif /* GIVPAINTERCAIROPIXBUF */
