#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "svgpath-to-cairo.h"
#include <fmt/core.h>
#include "cairo-flatten-by-bitmap.h"
#include <fstream>

using namespace fmt;
using namespace std;

static void die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap,fmt); 
    
    vfprintf(stderr, fmt, ap);
    exit(-1);
}

#define CASE(s) if (!strcmp(s, S_))

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

int main(int argc, char **argv)
{
  int argp=1;
  double resolution = 10; // Pixels per svg point

  if (argp >= argc)
    die("Need filename!\n");
  const char *filename = argv[argp++];

  cairo_surface_t *rec_surface = cairo_recording_surface_create(
    CAIRO_CONTENT_ALPHA,
    nullptr // unlimited extens
  );

  cairo_t *rec_cr = cairo_create(rec_surface);

  // Paint the path in white
  svgpaths_to_cairo(rec_cr, filename, true);

  cairo_path_t *path = cairo_copy_path_flat(rec_cr);
  print("num paths={}\n", path->num_data);
  path_to_giv(path, "/tmp/path.giv");

  cairo_fill(rec_cr);

  // Clean the rec_cr as we will reuse it. Is there another way?
  cairo_destroy(rec_cr);

  rec_cr = cairo_create(rec_surface);
  cairo_flatten_by_bitmap(rec_surface,
                          resolution,
                          // output
                          rec_cr);

  cairo_surface_t *surface = cairo_image_surface_create(
    CAIRO_FORMAT_ARGB32,
    1000,1000);
  cairo_t *cr = cairo_create(surface);
  
  cairo_set_source_surface (cr, rec_surface, 0.0, 0.0);
  cairo_set_source_rgb(rec_cr,1,1,1);
  cairo_paint (cr);
  cairo_destroy (cr);
  cairo_destroy (rec_cr);

  cairo_surface_write_to_png(surface, "/tmp/cairotest.png");
}
