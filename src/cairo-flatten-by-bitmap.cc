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
#include <spdlog/spdlog.h>

using namespace fmt;
using namespace std;

// Save a path to a giv file. This should perhaps be moved
// to a utility function.
static void path_to_giv(cairo_path_t *cpath,
                        const string& filename,
                        double resolution = 1.0,
                        const string& ref_image = string())
{
  ofstream fh(filename);
  if (ref_image.size())
    fh << format("$image {}\n", ref_image);

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
                       data[1].point.x*resolution,data[1].point.y*resolution);
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

// Take the cairo surface that may contain overlapping paths
// draw it to a bitmap and flatten it by tracing the bitmap.
// Write the result to the ctx object.
void FlattenByBitmap::flatten_by_bitmap(cairo_surface_t *rec_surface,
                                        double resolution)
{
  // Get bounding box
  double width=0, height=0, x0=0, y0=0;
  cairo_recording_surface_ink_extents (rec_surface,
                                       &x0, &y0,
                                       &width, &height);

  spdlog::info("flatten image by bitmap: rec_surface size: x0 y0 width height={} {} {} {}",
               x0, y0, width, height);

  // Modify x0 y0 width and height to add a bit of margin
  double margin = 10.0/resolution;
  width += 2*margin;
  height+= 2*margin;
  x0 -= margin;
  y0 -= margin;

  // Create an image with the above extents with resolution=resolution
  cairo_surface_t *surface = nullptr;
  spdlog::info("Initial resolution={}", resolution);
  while(1)
  {
    if (surface)
      cairo_surface_destroy(surface);
    surface = cairo_image_surface_create(
      CAIRO_FORMAT_A8,
      int(width*resolution),int(height*resolution));
    if (cairo_image_surface_get_width(surface))
      break;
    resolution/=2.0;
    spdlog::info("Reducing resolution to {}", resolution);
  }
  spdlog::info("surface size={}x{} resolution={}", 
               cairo_image_surface_get_width(surface),
               cairo_image_surface_get_height(surface),
               resolution);

  cairo_t *cr = cairo_create(surface);
  cairo_scale(cr, resolution, resolution);

  cairo_set_source_rgba(cr,0,0,0,0);
  cairo_paint (cr);
  // The surface is always painted in black. Not sure why...
  cairo_set_source_surface (cr, rec_surface, -x0, -y0);
  cairo_paint (cr);
  if (m_debug_dir.size())
  {
    string image_filename = format("{}/trace_input.png", m_debug_dir);
    cairo_surface_write_to_png(surface, image_filename.c_str());
    spdlog::info("saving to {}", image_filename);
  }
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

  // Single "holes" create bad artifacts. Close these by a one
  // step morphology!
  for (int row_idx=1; row_idx<surface_height-1; row_idx++)
  {
    uint8_t *prow = data+(row_idx-1)*surface_stride;
    uint8_t *row = data+row_idx*surface_stride;
    uint8_t *nrow = data+(row_idx+1)*surface_stride;
    for (int col_idx=1; col_idx<surface_width-1; col_idx++)
    {
      if (
         ~row[col_idx]
         & (
           // right
           (  prow[col_idx-1]             //  1 1 X
            & prow[col_idx]               //  1 0 X
            & row[col_idx-1]              //  1 1 X
            & nrow[col_idx-1]             //  (X=don't care)
            & nrow[col_idx])
           | //left
           (  prow[col_idx+1]             // X 1 1
            & prow[col_idx]               // X 0 1
            & row[col_idx+1]              // X 1 1
            & nrow[col_idx+1]
            & nrow[col_idx])
           | // up
           (  row[col_idx-1]              // X X X
            & row[col_idx+1]              // 1 0 1
            & nrow[col_idx-1]             // 1 1 1
            & nrow[col_idx]
            & nrow[col_idx+1])
           | // down
           (  prow[col_idx-1]             // 1 1 1
            & prow[col_idx]               // 1 0 1
            & prow[col_idx+1]             // X X X
            & row[col_idx-1]
            & row[col_idx+1])))
        row[col_idx]=255;                // Center 0 -> 1
    }
  }

#if 0
  ofstream fh("/tmp/flat-before-trace.pgm", ios::binary);
  fh << format("P5\n"
               "{} {}\n"
               "255\n",
               surface_stride, surface_height);
  fh.write((const char*)data, surface_height * surface_stride);
  fh.close();
#endif

  // And trace it and record the data in cr
  cairo_set_source_rgba(cr, 1,0,0,1);
  spdlog::info("trace image size={} {}", surface_width, surface_height);
  trace_image(surface_width,
              surface_height,
              surface_stride,
              data,
              resolution,
              // output
              m_ctx);
  //  cairo_fill(cr);

  if (m_debug_dir.size())
  {
    // trace input
    string image_filename = format("{}/trace_input_bin.png", m_debug_dir);
    cairo_surface_write_to_png(surface, image_filename.c_str());
    spdlog::info("saving to {}", image_filename);
    print("saving to {}\n", image_filename);

    // the resulting traced image
    cairo_path_t *path = cairo_copy_path_flat(m_ctx);
    print("num paths in cairo-flatten-by-bitmap ={}\n", path->num_data);
    string giv_filename = format("{}/{}", m_debug_dir, "path-by-bitmap.giv");
    spdlog::info("saving to {}", giv_filename);
    print("saving to {}\n", giv_filename);
    path_to_giv(path, giv_filename, resolution, image_filename);
    cairo_path_destroy(path);
  }

  cairo_destroy (cr);
  cairo_surface_destroy(surface);
  cairo_surface_destroy(traced_surface);
}
