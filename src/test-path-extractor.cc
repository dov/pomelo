// An example of how to use the path extractor library

#include "svgpath.h"
#include "cairocontext.h"
#include <stdarg.h>
#include <string>
#include <cairo.h>
#include <fmt/core.h>
#include "path-extractor.h"

using namespace tinyxml2;
using namespace std;
using namespace fmt;

static void die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap,fmt); 
    
    vfprintf(stderr, fmt, ap);
    exit(-1);
}

#define CASE(s) if (!strcmp(s, S_))

void draw(GraphicsContext *g, const std::string &path)
{
  SvgPath svgPath(path);
  svgPath.draw(g);
}

int main(int argc, char **argv)
{
  int argp = 1;

  if (argp >= argc)
    die("Need filename of svg file!\n");

  string filename = argv[argp++];

  PathExtractor pe(filename);

  print("path = {}\n", pe.GetPaths()[0].path);

  cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1000, 1000);
  cairo_t *cx = cairo_create(surface);
  cairo_set_source_rgb(cx,1,1,1);
  cairo_rectangle(cx,0,0,1000,1000);
  cairo_fill(cx);

  cairo_set_source_rgb(cx, 0.75, 0.34, 0.0);
  cairo_scale(cx,4,4);
  CairoContext graphicsContext(cx);
  draw(&graphicsContext, pe.GetPaths()[0].path);
  graphicsContext.fill();

  cairo_surface_write_to_png(surface, "/tmp/cairotest.png");
	
  cairo_destroy(cx);  
  cairo_surface_destroy(surface);
	
  exit(0);
}
