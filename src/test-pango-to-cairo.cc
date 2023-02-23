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
#include "cairo-flatten-by-bitmap.h"


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
  string font = "DejaVu Sans Bold 48";

  while(argp < argc && argv[argp][0] == '-') {
    char *S_ = argv[argp++];

    CASE("--help")
    {
      printf("test-pango-to-cairo - Test converting a string to cairo (with tracing)\n\n"
             "Syntax:\n"
             "    test-pango-to-cairo [--font font] markup\n"
             "\n"
             "Options:\n"
             "    --font font    The font\n");
      exit(0);
    }
    CASE("--font")
    {
      font = argv[argp++];
      continue;
    }
    die("Unknown option %s!\n", S_);
  }
  

  if (argp >= argc)
    die("Need markup!\n");
  const char *markup = argv[argp++];

  cairo_surface_t *rec_surface = cairo_recording_surface_create(
    CAIRO_CONTENT_ALPHA,
    nullptr // unlimited extens
  );

  cairo_t *rec_cr = cairo_create(rec_surface);

#if 0
  cairo_set_source_rgb(rec_cr,1,1,1);
  cairo_paint (rec_cr);
#endif

  cairo_set_source_rgb(rec_cr, 0.75, 0.34, 0.0);

  PangoFontDescription *desc = pango_font_description_from_string(font.c_str());
  pangomarkup_to_cairo(rec_cr, markup, desc);
  pango_font_description_free(desc);
  cairo_set_tolerance(rec_cr, 0.1); 

  cairo_path_t *path = cairo_copy_path_flat(rec_cr);
  print("num paths={}\n", path->num_data);

  path_to_giv(path, "/tmp/path.giv");
  cairo_path_destroy(path);

  double resolution = 10;
  cairo_fill(rec_cr);

  cairo_destroy(rec_cr);
  rec_cr = cairo_create(rec_surface);
  
  cairo_flatten_by_bitmap(rec_surface,
                          resolution,
                          // output
                          rec_cr);

  cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                                        5000,500);
  cairo_t *cr = cairo_create(surface);
  cairo_set_source_surface (cr, rec_surface, 0.0, 0.0);
  cairo_paint (cr);

  path = cairo_copy_path_flat(cr);
  print("num paths after image={}\n", path->num_data);
  path_to_giv(path, "/tmp/path-after-image.giv");

  cairo_fill(cr);
  cairo_destroy (cr);
  

  // Use this for the pangacairo example
  //  draw_text(cr);


  cairo_surface_write_to_png(surface, "/tmp/cairotest.png");
  cairo_surface_destroy(surface);
}
