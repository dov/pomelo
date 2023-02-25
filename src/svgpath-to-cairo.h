//======================================================================
//  svgpath-to-cairo.h - Turn a svg path into a cairo context
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sat Jan 30 19:13:34 2021
//----------------------------------------------------------------------
#ifndef SVGPATH_TO_CAIRO_H
#define SVGPATH_TO_CAIRO_H

#include <string>
#include <cairo/cairo.h>

class SvgPathsToCairo {
 public:
  SvgPathsToCairo(cairo_t *ctx) : m_ctx(ctx) {}

  void set_debug_dir(const std::string& debug_dir) {
    m_debug_dir = debug_dir;
  }

  // svgpaths_to_cairo reads a svg document and adds its contents
  // to the cairo path.
  //
  // keep paths means that we will not consume any paths by fill
  // or strok.
  void parse_file(const std::string& filename,
                  bool keep_paths=false);

 private:
  cairo_t *m_ctx;
  std::string m_debug_dir;
};

#endif /* SVGPATH-TO-CAIRO */
