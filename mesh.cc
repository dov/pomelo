//======================================================================
//  mesh.h - A simple mesh interface for saving to STL.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Mon Oct 12 22:10:45 2020
//----------------------------------------------------------------------
#include "mesh.h"
#include <fstream>
#include <iostream>
#include "fmt/core.h"

using namespace fmt;
using namespace std;
using namespace glm;

// Read and return a mesh
std::shared_ptr<Mesh> read_stl(const std::string& filename)
{
  std::shared_ptr<Mesh> mesh = make_shared<Mesh>();
  // Shortcuts
  auto& vertices = mesh->vertices;

  ifstream in(filename, ios::binary);
  if (!in.good())
    throw runtime_error("Failed opening " + std::string(filename) + "!");

  char title[80];
  std::string s0, s1;
  float nx,ny,nz;
  float v1x,v1y,v1z,v2x,v2y,v2z,v3x,v3y,v3z;
  bool do_binary = false;

  in.getline(title, sizeof(title));
  if (strncmp(title,"solid",5)==0) {      
    while (!in.eof()) {
      in >> s0;                                // facet || endsolid
      if (s0=="facet") {
        in >> s1 >> nx >> ny >> nz;            // normal x y z
        in >> s0 >> s1;                        // outer loop
        in >> s0 >> v1x >> v1y >> v1z;         // vertex x y z
        in >> s0 >> v2x >> v2y >> v2z;         // vertex x y z
        in >> s0 >> v3x >> v3y >> v3z;         // vertex x y z
        in >> s0;                            // endloop
        in >> s0;                            // endfacet
  
        vec3 v1 { v1x,v1y,v1z };
        vec3 v2 { v2x,v2y,v2z };
        vec3 v3 { v3x,v3y,v3z } ;
  
        // Generate a new Triangle with Normal as 3 Vertices
        vertices.push_back(v1);
        vertices.push_back(v2);
        vertices.push_back(v3);
      }
      else if (s0=="endsolid") {
        break;
      }
      else
      {
        // retry as binary
        do_binary = true;
        break;
      }
    }
  }
  else
    do_binary = true;

  if (do_binary)
  {
    FILE *f = fopen(filename.c_str(), "rb");
    if (!f)
      throw runtime_error(string("Failed opening file ") + filename + "!");
    char title[80];
    int num_faces;
    fread(title, 80, 1, f);
    fread((void*)&num_faces, 4, 1, f);
    float v[12]; // normal=3, vertices=3*3 = 12
    short attribute;   // Used for color like VisCam and SolidView
    vertices.reserve(num_faces*3);

    // Bin stl: Normal(3*float), Vertices(9*float), 2 Bytes Spacer
    for (int i=0; i<(int)num_faces; ++i) {
      fread((void*)&v[0], sizeof(float), 12, f);
      fread((void*)&attribute, 2, 1, f); // space

      vec3 v1 { v[3],v[4],v[5] };
      vec3 v2 { v[6],v[7],v[8] };
      vec3 v3 { v[9],v[10],v[11] };
      // Generate a new Triangle with Normal as 3 Vertices
      vertices.push_back(v1);
      vertices.push_back(v2);
      vertices.push_back(v3);
    }
    fclose(f);      
  }
  print("{} faces\n", (int)vertices.size()/3);
  in.close();

  return mesh;
}

