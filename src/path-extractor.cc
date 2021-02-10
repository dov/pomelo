//======================================================================
//  path-extractor.cc - Extract the paths from an svg file
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Fri Jan 29 12:38:42 2021
//----------------------------------------------------------------------

#include "path-extractor.h"
#include <fmt/core.h>
#include "path-extractor.h"
#include "svgpath.h"
#include "cairocontext.h"
#include "tinyxml2.h"

using namespace tinyxml2;
using namespace std;
using namespace fmt;


PathExtractor::PathExtractor(const string& SvgFilename)
{
  XMLDocument doc;
  doc.LoadFile(SvgFilename.c_str());

  // Recursively look for paths
  Traverse(doc.FirstChildElement());
}

// Recursive traverse the xml tree
void PathExtractor::Traverse(XMLElement *element)
{
  for (XMLElement* el = element->FirstChildElement(); el; el = el->NextSiblingElement())
    {
      XMLElement* el_save = el;

      if (string(el->Name()) == "path")
        {
          const char *svg_path;
          el->QueryStringAttribute("d", &svg_path);
          this->paths.push_back({svg_path, cairo_matrix_t()});
        }

      if (!el_save->NoChildren())
        Traverse(el_save);
    }
}

