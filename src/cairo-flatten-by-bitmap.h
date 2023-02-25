//======================================================================
//  cairo_flatten_by_bitmap.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sun Feb 19 21:13:37 2023
//----------------------------------------------------------------------
#ifndef CAIRO_FLATTEN_BY_BITMAP_H
#define CAIRO_FLATTEN_BY_BITMAP_H

#include <cairo/cairo.h>

class FlattenByBitmap {
 public:
 FlattenByBitmap(cairo_t *ctx) : m_ctx(ctx) {}

  void set_debug_dir(const std::string& debug_dir) {
    m_debug_dir = debug_dir;
  }

  // Take the cairo surface that may contain overlapping paths
  // draw it to a bitmap and flatten it by tracing the bitmap.
  // Write the result to the ctx object.
  void flatten_by_bitmap(cairo_surface_t *rec_surface,
                         double resolution);


 private:
  std::string m_debug_dir;
  cairo_t *m_ctx;
};


#endif /* CAIRO_FLATTEN_BY_BITMAP */
