//======================================================================
//  smooth-sharp-angles.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Mon Feb  8 15:11:04 2021
//----------------------------------------------------------------------
#ifndef SMOOTH_SHARP_ANGLES_H
#define SMOOTH_SHARP_ANGLES_H

#include "textrusion.h"

std::vector<Polygon_with_holes>
smooth_acute_angles(double radius,
                    double max_angle_to_smooth,
                    const std::vector<Polygon_with_holes>& hpolys,
                    int num_smooth_points);


#endif /* SMOOTH-SHARP-ANGLES */
