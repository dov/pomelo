//======================================================================
//  mesh-utils.h - Utilities for manipulating meshes
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Thu Jan 30 22:30:25 2025
//----------------------------------------------------------------------
#ifndef MESH_UTILS_H
#define MESH_UTILS_H

#include <mesh.h>

// Fold near vertices into a common vertex points
void merge_by_distance(Mesh& mesh, double merge_distance);

// For debugging. Count the number of unique vertices
int count_unique_vertices(const Mesh& mesh);

#endif /* MESH-UTILS */
