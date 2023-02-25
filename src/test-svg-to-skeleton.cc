  
//======================================================================
//  test-svg-to-skeleton.cpp - A test program for the generating
//  the skeleton.
//   
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sat Feb  6 20:51:21 2021
//----------------------------------------------------------------------

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <textrusion.h>
#include <smooth-sharp-angles.h>
#include <fmt/core.h>
#include "cairo-flatten-by-bitmap.h"


using namespace std;
using namespace Glib;
using namespace fmt;

const double MY_PI=3.141592653589793;
const double DEG2RAD=MY_PI/180;

static void die(const char *ffmt, ...)
{
  va_list ap;
  va_start(ap,ffmt); 
    
  vfprintf(stderr, ffmt, ap);
  exit(-1);
}

#define CASE(s) if (!strcmp(s, S_))

static void save_poly_to_giv(ustring filename,
                  ustring header,
                  Polygon_2 poly,
                  bool append)
{
    auto flags = ofstream::out;
    if (append)
        flags |= ofstream::app;
    ofstream of(filename, flags);
    of << header;
    of << "m ";
    for (const auto& p  : poly)
        of << p.x() << " " << p.y() << "\n";
    of << "z\n\n";
    of.close();
}
                     

int main(int argc, char **argv)
{
  int argp = 1;
  string giv_filename = "skeleton.giv";
  double max_angle_to_smooth = 135*DEG2RAD;
  bool do_smooth = false;
  double linear_limit = 500;

  while(argp < argc && argv[argp][0] == '-') {
    char *S_ = argv[argp++];

    CASE("--help") {
      printf("test-svg-skeleton - Test the skeleton library\n\n"
             "\n"
             "Description:\n"
             "   Tests the skeleton library with smoothing. Writes the\n"
             "   result to a giv file\n\n"
             "Syntax:\n"
             "    test-svg-skeleton ...\n"
             "\n"
             "Options:\n"
             "    -o   fn                 Set giv filename output (Default is skeleton.giv)\n"
             "    --smooth                Smooth the angles. Default is not to smooth\n"
             "    --linear_limit ll       Linear limit. Default is 500\n"
             "    --max_angle max_angle   Max angle to smooth\n"
             );
      exit(0);
    }
    CASE("--smooth")
      {
        do_smooth =true;
        continue;
      }
    CASE("--linear_limit")
      {
        linear_limit = atof(argv[argp++]);
        continue;
      }
    CASE("--max_angle")
      {
        max_angle_to_smooth = atoi(argv[argp++]) * DEG2RAD;
        continue;
      }
    CASE("-o")
      {
        giv_filename = argv[argp++];
        continue;
      }
      
    die("Unknown option %s!\n", S_);
  }

  if (argp>= argc)
    die("Need svg filename!\n");

  const char* filename = argv[argp++];

  TeXtrusion textrusion;
  textrusion.linear_limit = linear_limit;
  double resolution = 10;
  
  Cairo::RefPtr<Cairo::Surface> surface = Cairo::RecordingSurface::create();
  textrusion.svg_filename_to_context(surface, filename);

  Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surface);
  FlattenByBitmap fb(cr->cobj());
  fb.set_debug_dir("/tmp");
  fb.flatten_by_bitmap(surface->cobj(),
                       resolution);

  auto poly = textrusion.cairo_path_to_polygons(cr);
  auto hpolys = textrusion.polys_to_polys_with_holes(poly);

  int hp_idx=-1;
  for (auto& hp : hpolys)
    {
      hp_idx++;

      save_poly_to_giv("hp.giv",
                  format("$path {}/outer\n"
                         "$color red\n"
                         , hp_idx),
                       hp.outer_boundary(),
                  hp_idx>0);
      int hole_idx=-1;
      for (auto h : hp.holes())
        {
          hole_idx++;
          save_poly_to_giv("hp.giv",
                           format("$path {}/hole {}\n"
                                  "$color green\n"
                                  , hp_idx, hole_idx),
                           h,
                           true);
        }
    }

  if (do_smooth)
    hpolys = smooth_acute_angles(0.5, max_angle_to_smooth, hpolys, 16);

  string giv_string;
  auto skel = textrusion.skeletonize(hpolys,
                                     // output
                                     giv_string);
  print("Saving skeleton to {}\n", giv_filename);
  ofstream fh(giv_filename);
  fh << giv_string;
  fh.close();
  
  exit(0);
  return(0);
}
