//======================================================================
//  path-extractor.h - Extract the paths from an svg file
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Fri Jan 29 12:38:42 2021
//----------------------------------------------------------------------
#ifndef PATH_EXTRACTOR_H
#define PATH_EXTRACTOR_H

#include <cairo.h>
#include <string>
#include <vector>

namespace tinyxml2 {
  class XMLElement;
}

// An svg path with its total concatenated cairo transformation
struct PathTrans {
  std::string path;
  cairo_matrix_t trans;
};

// The path extractor uses the tinyxml2 library to extract all paths
// in the svg document. Currently it ignores surrounding transformations,
// and thus assumes that the document has been fully ungrouped.
class PathExtractor
{
  public:
  PathExtractor(const std::string& SvgFilename);

  std::vector<PathTrans> GetPaths() const { return paths; }

  private:
  void ParseSvg();

  // Traverse the xml tree
  void Traverse(tinyxml2::XMLElement * element);

  std::vector<PathTrans> paths;
};

#endif /* PATH-EXTRACTOR */
