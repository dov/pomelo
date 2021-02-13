#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "svgpath-to-cairo.h"
#include <fmt/core.h>

using namespace fmt;


static void die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap,fmt); 
    
    vfprintf(stderr, fmt, ap);
    exit(-1);
}

#define CASE(s) if (!strcmp(s, S_))

int main(int argc, char **argv)
{
  int argp=1;

  if (argp >= argc)
    die("Need filename!\n");
  const char *filename = argv[argp++];

  cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                                        1000,1000);
  cairo_t *cr = cairo_create(surface);

  svgpaths_to_cairo(cr, filename);

  cairo_path_t *path = cairo_copy_path_flat(cr);
  print("num paths={}\n", path->num_data);

  cairo_surface_write_to_png(surface, "/tmp/cairotest.png");
}
