// Test the pango-to-cairo functionality
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <pango/pangocairo.h>
#include <math.h>
#include "pango-to-cairo.h"
#include <fmt/core.h>
#include <pangomm/init.h>
#include <fstream>

using namespace std;
using namespace fmt;

static void die(const char *fmt, ...)
{
  va_list ap;
  va_start(ap,fmt); 
    
  vfprintf(stderr, fmt, ap);
  exit(-1);
}

// Save a path to a giv file. This should perhaps be moved
// to a utility function.
void path_to_giv(cairo_path_t *cpath,
                 const char *filename)
{
  ofstream fh(filename);
  print("num_data = {}\n", cpath->num_data);
  for (int i=0; i < cpath->num_data; i += cpath->data[i].header.length)
    {
      auto data = &cpath->data[i];
      switch (data->header.type)
        {
        case CAIRO_PATH_MOVE_TO:
        case CAIRO_PATH_LINE_TO:
          if (data->header.type==CAIRO_PATH_MOVE_TO)
            fh << "m ";
          fh << format("{} {}\n",
                       data[1].point.x,data[1].point.y);
          break;
        case CAIRO_PATH_CURVE_TO:
          // No curve support. Just draw a line to last point
          printf("Warning! We shouldn't be here! We should be flattened!!\n");
          break;
        case CAIRO_PATH_CLOSE_PATH:
          fh << "z\n";
        }
    }
}

#define CASE(s) if (!strcmp(s, S_))

int main(int argc, char **argv)
{
  int argp=1;

  if (argp >= argc)
    die("Need markup!\n");
  const char *markup = argv[argp++];

  cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                                        5000,500);
  cairo_t *cr = cairo_create(surface);

  cairo_set_source_rgb(cr,1,1,1);
  cairo_paint (cr);

  cairo_set_source_rgb(cr, 0.75, 0.34, 0.0);

  PangoFontDescription *desc = pango_font_description_from_string("DejaVu Sans Bold 48");
  pangomarkup_to_cairo(cr, markup, desc);
  pango_font_description_free(desc);
  cairo_set_tolerance(cr, 0.1); 

  cairo_path_t *path = cairo_copy_path_flat(cr);
  print("num paths={}\n", path->num_data);

  path_to_giv(path, "/tmp/path.giv");
  cairo_path_destroy(path);
  cairo_fill(cr);

  // Use this for the pangacairo example
  //  draw_text(cr);


  cairo_surface_write_to_png(surface, "/tmp/cairotest.png");
  cairo_surface_destroy(surface);
}
