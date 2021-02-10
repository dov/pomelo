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
#include <pangocairo-to-contour.h>
#include <smooth-sharp-angles.h>

using namespace std;

const double MY_PI=3.141592653589793;
const double DEG2RAD=MY_PI/180;

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
  int argp = 1;
  string giv_filename = "skeleton.giv";
  double max_angle_to_smooth = 135*DEG2RAD;

  while(argp < argc && argv[argp][0] == '-') {
    char *S_ = argv[argp++];

    CASE("--help") {
      printf("test-svg-skeleton - Test the skeleton library\n\n"
             "Syntax:\n"
             "    test-svg-skeleton ...\n"
             "\n"
             "Options:\n"
             "    -o   fn                 Set giv filename output\n"
             "    --max_anlge max_angle   Max angle to smooth\n"
             );
      exit(0);
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
  auto cr = textrusion.svg_filename_to_context(filename);
  auto poly = textrusion.cairo_path_to_polygons(cr);
  auto hpolys = textrusion.polys_to_polys_with_holes(poly);
  hpolys = smooth_acute_angles(0.5, max_angle_to_smooth, hpolys, 16);
  string giv_string;
  auto skel = textrusion.skeletonize(hpolys,
                                     // output
                                     giv_string);
  ofstream fh(giv_filename);
  fh << giv_string;
  fh.close();
  
  exit(0);
  return(0);
}
