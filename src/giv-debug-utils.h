//======================================================================
//  giv-debug-utils.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Wed Oct 23 07:12:57 2024
//----------------------------------------------------------------------
#ifndef GIV_DEBUG_UTILS_H
#define GIV_DEBUG_UTILS_H

#include <string>
#include <textrusion.h>

void poly_to_giv(const std::string& filename,
                 const std::string& header,
                 const Polygon_2& poly,
                 bool append);

void polys_with_holes_to_giv(const std::string& filename,
                             const std::string& outer_header,
                             const std::string& hole_header,
                             std::vector<Polygon_with_holes>& polys,
                             bool append);

#endif /* GIV-DEBUG-UTILS */
