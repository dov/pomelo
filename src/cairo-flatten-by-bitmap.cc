//======================================================================
//  cairo_flatten_by_bitmap.cpp - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sun Feb 19 21:15:47 2023
//----------------------------------------------------------------------

#include <fmt/core.h>
#include <fstream>
#include "cairo-flatten-by-bitmap.h"
#include "image-tracer-bw.h"

using namespace fmt;
using namespace std;

// Save a path to a giv file. This should perhaps be moved
// to a utility function.
static void path_to_giv(cairo_path_t *cpath,
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

// Take the cairo context that may contain overlapping paths
// draw it to a bitmap and flatten it by tracing the bitmap.
void cairo_flatten_by_bitmap(cairo_surface_t *rec_surface,
                             double resolution,
                             // output
                             cairo_t *res
                             )
{
  // Get bounding box
  double width=0, height=0, x0=0, y0=0;
  cairo_recording_surface_ink_extents (rec_surface,
                                       &x0, &y0,
                                       &width, &height);

  print("Without margins: x0 y0 width height={} {} {} {}\n",
        x0, y0, width, height);

  // Modify x0 y0 width and height to add a bit of margin
  double margin = 10.0/resolution;
  width += 2*margin;
  height+= 2*margin;
  x0 -= margin;
  y0 -= margin;

  print("With margins: x0 y0 width height={} {} {} {}\n",
        x0, y0, width, height);
    
  // Create an image with the above extents with resolution=resolution
  cairo_surface_t *surface = cairo_image_surface_create(
    CAIRO_FORMAT_A8,
    int(width*resolution),int(height*resolution));
  cairo_t *cr = cairo_create(surface);
  cairo_scale(cr, resolution, resolution);

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

  // Recreate the surface and paint the cairo context in it
  cairo_surface_t *traced_surface = cairo_image_surface_create(
    CAIRO_FORMAT_ARGB32,
    int(width*resolution),int(height*resolution));
  cr = cairo_create(traced_surface);
  cairo_set_source_rgba(cr, 0,1,0,1);
  cairo_paint(cr);

  // Binarize the image
  for (int i=0; i<surface_stride*surface_height; i++)
    data[i] = int(data[i]>128)*255;


  ofstream fh("/tmp/flat-before-trace.pgm", ios::binary);
  fh << format("P5\n"
               "{} {}\n"
               "255\n",
               surface_stride, surface_height);
  fh.write((const char*)data, surface_height * surface_stride);
  fh.close();

  cairo_surface_write_to_png(surface, "/tmp/flat-by-image.png");

  // And trace it and record the data in cr
  cairo_set_source_rgba(cr, 1,0,0,1);
  print("trace image size={} {}\n", surface_width, surface_height);
  trace_image(surface_width,
              surface_height,
              surface_stride,
              data,
              resolution,
              // output
              res);
  //  cairo_fill(cr);

  {
    cairo_path_t *path = cairo_copy_path_flat(res);
    print("num paths in cairo-flatten-by-bitmap ={}\n", path->num_data);
  
    print("saving to /tmp/path-by-bitmap.giv\n");
    path_to_giv(path, "/tmp/path-by-bitmap.giv");
  }

  //  cairo_surface_write_to_png(traced_surface, "/tmp/flat-repainted.png");
  cairo_destroy (cr);
  cairo_surface_destroy(surface);
  cairo_surface_destroy(traced_surface);
}

