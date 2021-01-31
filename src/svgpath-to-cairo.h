//======================================================================
//  svgpath-to-cairo.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sat Jan 30 19:13:34 2021
//----------------------------------------------------------------------
#ifndef SVGPATH_TO_CAIRO_H
#define SVGPATH_TO_CAIRO_H

#include <cairo/cairo.h>

// svgpaths_to_cairo reads a svg document and adds its contents
// to the cairo path.
//
// keep paths means that we will not consume any paths by fill
// or strok.
void svgpaths_to_cairo(cairo_t *ctx,       
                       const char *filename,
                       bool keep_paths=false);

#endif /* SVGPATH-TO-CAIRO */
