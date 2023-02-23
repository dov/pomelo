//======================================================================
//  image-tracer-bw.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Mon Feb 20 22:58:03 2023
//----------------------------------------------------------------------
#ifndef IMAGE_TRACER_BW_H
#define IMAGE_TRACER_BW_H

#include <cairo/cairo.h>

// External entry point. Receive's an image and traces it to to cr.
void trace_image(int width,
                 int height,
                 int stride,
                 uint8_t *data,
                 double resolution, // Image will be inverse scaled by this
                 // output
                 cairo_t *cr);

#endif /* IMAGE-TRACER-BW */
