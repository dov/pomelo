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
  std::vector<glm::vec3> vertices;  // 3 at a time for triangles
};

std::shared_ptr<Mesh> read_stl(const std::string& filename);
void save_stl(std::shared_ptr<Mesh> mesh, const std::string& filename);
void save_gltf(std::shared_ptr<Mesh> mesh, const std::string& filename);

#endif /* MESH */
