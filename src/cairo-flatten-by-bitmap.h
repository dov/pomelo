//======================================================================
//  cairo_flatten_by_bitmap.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sun Feb 19 21:13:37 2023
//----------------------------------------------------------------------
#ifndef CAIRO_FLATTEN_BY_BITMAP_H
#define CAIRO_FLATTEN_BY_BITMAP_H

#include <cairo/cairo.h>

// Take the cairo context that may contain overlapping paths
// draw it to a bitmap and flatten it by tracing the bitmap.
cairo_t *cairo_flatten_by_bitmap(cairo_surface_t *rec_surface,
                                 double resolution);

#endif /* CAIRO_FLATTEN_BY_BITMAP */
