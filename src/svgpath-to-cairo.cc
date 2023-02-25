// Uses nanosvg to add the svg paths from an svg file to
// a cairo context.
#define NANOSVG_IMPLEMENTATION	// Expands implementation
#include <stdio.h>
#include "nanosvg.h"
#include "cairo/cairo.h"
#include <fmt/core.h>
#include "svgpath-to-cairo.h"

using namespace fmt;
using namespace std;

// For pomelo we don't need to set the color, but it is nice for
// debugging.

// Set a nanosvg color in the cairo context
static void set_ncolor(cairo_t *ctx, unsigned int c)
{
  cairo_set_source_rgba(ctx,
                        ((c&0xff))/256.0,
                        ((c>>8)&0xff)/256.0,
                        ((c>>16)&0xff)/256.0,
                        (c>>24)/256.0
                        );
}

// to the cairo path.
static void svgpaths_to_cairo(cairo_t *cr,
                              const char *filename,
                              bool keep_paths)
{
  struct NSVGimage* image;
  image = nsvgParseFromFile(filename, "px", 96);

  int num_shapes = 0;
  for (auto shape = image->shapes; shape != NULL; shape = shape->next)
    {
      num_shapes++;
      if (!(shape->flags & NSVG_FLAGS_VISIBLE))
        continue;

      int num_paths = 0;
      for (auto path = shape->paths; path != NULL; path = path->next)
        {
          num_paths++;
          float* p = path->pts;
          cairo_move_to(cr, p[0], p[1]);
          for (int i = 0; i < path->npts-1; i += 3)
            {
              p = &path->pts[i*2];
              cairo_curve_to(cr,p[2],p[3],p[4],p[5],p[6],p[7]);
            }
          if (path->closed)
            cairo_close_path(cr);
        }

      if (!keep_paths)
        {
          if (shape->stroke.type != NSVG_PAINT_NONE)
            {
              set_ncolor(cr,shape->stroke.color);
              cairo_set_line_width(cr,shape->strokeWidth);
              if (shape->fill.type != NSVG_PAINT_NONE)
                cairo_stroke_preserve(cr);
              else
                cairo_stroke(cr);
            }
          if (shape->fill.type != NSVG_PAINT_NONE)
            {
              set_ncolor(cr,shape->fill.color);
              cairo_fill(cr);
            }
        }
    }

  // Delete
  nsvgDelete(image);
}

// c++ front end
void SvgPathsToCairo::parse_file(const string& filename,
                                 bool keep_paths)
{
  svgpaths_to_cairo(m_ctx,       
                    filename.c_str(),
                    keep_paths);
}
