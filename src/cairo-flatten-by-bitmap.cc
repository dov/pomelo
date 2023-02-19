//======================================================================
//  cairo_flatten_by_bitmap.cpp - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sun Feb 19 21:15:47 2023
//----------------------------------------------------------------------

#include <fmt/core.h>
#include <fstream>
#include "cairo-flatten-by-bitmap.h"

using namespace fmt;
using namespace std;

// Take the cairo context that may contain overlapping paths
// draw it to a bitmap and flatten it by tracing the bitmap.
cairo_t *cairo_flatten_by_bitmap(cairo_surface_t *rec_surface,
                                 double resolution)
{
  cairo_t *res = cairo_create(rec_surface);

  // Get bounding box
  double width=0, height=0, x0=0, y0=0;
  cairo_recording_surface_ink_extents (rec_surface,
                                       &x0, &y0,
                                       &width, &height);

  // Modify x0 y0 width and height to add a bit of margin
  double margin = 5/resolution;
  width += 2*margin;
  height+= 2*margin;
  x0 -= margin;
  y0 -= margin;

  print("x0 y0 width height={} {} {} {}\n",
        x0, y0, width, height);
    
  // Create an image with the above extents with resolution=resolution
  cairo_surface_t *surface = cairo_image_surface_create(
    CAIRO_FORMAT_A8,
    int(width*resolution),int(height*resolution));
  cairo_t *cr = cairo_create(surface);

  cairo_set_source_rgba(cr,0,0,0,0);
  cairo_paint (cr);
  // The surface is always painted in black. Not sure why...
  cairo_set_source_surface (cr, rec_surface, -x0, -y0);
  cairo_paint (cr);
  cairo_surface_write_to_png(surface, "/tmp/flat-by-image.png");
  cairo_destroy (cr);

  // Get surface to my own structure in preparation of tracing
  uint8_t *data = cairo_image_surface_get_data(surface);
  int surface_width = cairo_image_surface_get_width(surface);
  int surface_height = cairo_image_surface_get_height(surface);
  int surface_stride = cairo_image_surface_get_stride(surface);

  ofstream fh("/tmp/flat-by-image.pgm", ios::binary);
  fh << format("P5\n"
               "{} {}\n"
               "255\n",
               surface_stride, surface_height);
  fh.write((const char*)data, surface_height * surface_stride);
  fh.close();

  return res;
}

