//======================================================================
// Debug utils for saving algos to giv.
//
// 2024-10-23 Wed
// Dov Grobgeld <dov.grobgeld@gmail.com>
//----------------------------------------------------------------------

#include <fstream>
#include "giv-debug-utils.h"

using namespace std;

void poly_to_giv(const string& filename,
                 const string& header,
                 const Polygon_2& poly,
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


void polys_with_holes_to_giv(const string& filename,
                             const string& outer_header,
                             const string& hole_header,
                             std::vector<Polygon_with_holes>& polys,
                             bool append)
{
  for (const auto &ph : polys)
  {
    poly_to_giv(filename,
                outer_header,
                ph.outer_boundary(),
                append);
    append = true;
    for (const auto &h : ph.holes())
      poly_to_giv(filename,
                  hole_header,
                  h,
                  append);
      
  }
}

