//======================================================================
//  mesh.h - A simple mesh interface for saving to STL.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Mon Oct 12 22:10:45 2020
//----------------------------------------------------------------------
#ifndef MESH_H
#define MESH_H

#include <memory>
#include <array>
#include <vector>
#include <glm/vec3.hpp>

class Mesh {
  public:
  std::vector<glm::dvec3> vertices;  // 3 at a time for triangles
  glm::vec3 color;
};

class MultiMesh : public std::vector<Mesh> {
  public:
  // TBD
  void save_gltf(const std::string& filename);
};

std::shared_ptr<Mesh> read_stl(const std::string& filename);

void save_stl(const Mesh& mesh, const std::string& filename);

#if 0
void save_gltf(std::shared_ptr<Mesh> mesh,
               const std::string& filename,
               glm::vec3 mesh_colors);
#endif
#endif /* MESH */
