//======================================================================
//  pango-to-cairo.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sat Feb 13 07:24:42 2021
//----------------------------------------------------------------------
#ifndef PANGO_TO_CAIRO_H
#define PANGO_TO_CAIRO_H

#include <cairo/cairo.h>
#include <pango/pango.h>

// pangomarkup_to_cairo renders the text into the cairo context.
void pangomarkup_to_cairo(cairo_t *cr,       
                          const char *markup,
                          PangoFontDescription *desc);


#endif /* PANGO-TO-CAIRO */
