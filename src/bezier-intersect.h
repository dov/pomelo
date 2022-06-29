//======================================================================
//  bezier-intersect.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Fri Jun 24 08:55:02 2022
//----------------------------------------------------------------------
#ifndef BEZIER_INTERSECT_H
#define BEZIER_INTERSECT_H

#include <vector>

namespace bezier_intersect {
  
class Vec2 {
  public:
  double x, y;
};

class Bezier {
  public:
  Vec2 xy0, cp0, cp1, xy1;
};

class Line {
  public:
  Vec2 xy0;
  Vec2 xy1;
};

// Find all intersections between a bezier and a line
std::vector<Vec2> find_bezier_line_intersection(const Bezier& bezier,
                                                const Line& line);


};

#endif /* BEZIER_INTERSECT */
