//======================================================================
//  mesh.h - A simple mesh interface for saving to STL and gltf.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Mon Oct 12 22:10:45 2020
//----------------------------------------------------------------------
#include "mesh.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include "fmt/core.h"
#include "spdlog/spdlog.h"


// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tiny_gltf.h"

using namespace std;
using namespace glm;
using fmt::print;
namespace fs = std::filesystem;

constexpr float export_scale = 0.01;

// Loop over the coordinates in the mesh and zero out all coordinates
// than are smaller than 1+epsilon where epsilon is with respect to a
// a float.
static float zero_small_val(float v)
{
  if (fabs(v) < 1e-6)
    return 0;
  return v;
}

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
  fmt::print("{} faces\n", (int)vertices.size()/3);
  in.close();

  return mesh;
}

// 3D Mesh routines
void save_stl(const Mesh& mesh,
              const std::string& filename)
{
  // Write header
  const int header_size = 80;
  char header[header_size];
  std::fill_n(header, header_size, 0);
  ofstream fh(filename,ofstream::binary|ofstream::out);
  
  char header_string[] = "* Created by Pomelo *";
  memcpy(header, header_string, sizeof(header_string));

  fh.write(header, header_size);

  // Make use of the fact that all the vertices are ordered in
  // groups of three composing the contained triangles.
  const  auto& vertices = mesh.vertices; // shortcut

  // Copy the non-zero triangles
  vector<glm::dvec3> non_zero_vertices;
  for(size_t tr_idx=0; tr_idx<mesh.vertices.size()/3; tr_idx++)
    {
      bool zero_length_edge = false;
      for (int i=0; i<3; i++)
        {
          // Check if the length of the edge is zero
          double len = glm::length(vertices[tr_idx*3+i]-vertices[tr_idx*3+(i+1)%3]);
          if (len < 1e-9)
            {
              spdlog::warn("Warning: Zero length edge in triangle {}. len={}\n", tr_idx, len);
              zero_length_edge = true;
              break;
            }
        }
#if 0
      if (!zero_length_edge) // Change back!
        continue;
#endif
      for (int i=0; i<3; i++)
        non_zero_vertices.push_back(vertices[tr_idx*3+i]);
    }

  size_t size = (size_t)non_zero_vertices.size()/3;
  fh.write((const char*)&size, 4);

  uint16_t color=0;
  for(size_t tr_idx=0; tr_idx<size; tr_idx++)
    {
      float fzero {0};
      for (int i=0; i<3; i++)
        fh.write((const char*)&fzero,4);

      for (int i=0; i<3; i++)
        {
          for (int j=0; j<3; j++)
            {
              float f = (float)non_zero_vertices[tr_idx*3+i][j] * export_scale;
              f = zero_small_val(f);
              fh.write((char*)&f, 4);
            }
        }
      fh.write((char*)&color, 2);
    }
  fh.close();
}


void MultiMesh::save_gltf(const std::string& filename)
{
  // Create a model with a single mesh and save it as a gltf file
  tinygltf::Model m;
  tinygltf::Scene scene;
  tinygltf::Mesh tmesh;
  tinygltf::Primitive primitive;
  tinygltf::Node node;
  tinygltf::Buffer buffer;
  tinygltf::BufferView bufferView1;
  tinygltf::BufferView bufferView2;
  tinygltf::Accessor accessor1;
  tinygltf::Accessor accessor2;
  tinygltf::Asset asset;


  tinygltf::Light light;
  light.color = {1.0f, 1.0f, 1.0f};
  light.intensity = 1.0f;
  light.range = 10.0f;
  light.type = "directional";
  m.lights.push_back(light);
  
  // Create a buffer that contain the vertices and faces for
  // all the meshes
  int buffer_size = 0;
  for (const auto &m : *this)
  {
    const auto& vertices = m.vertices; // shortcut
    int num_vertices = vertices.size();
    int num_faces = num_vertices/3;
    buffer_size += (num_faces * 3 * sizeof(int32_t)
                    + num_vertices * 3 * sizeof(float));
  }
  
  buffer.data.resize(buffer_size);

  // The limits for each mesh
  vector<vector<double>> minValues(
    this->size(),
    {numeric_limits<float>::max(),
     numeric_limits<float>::max(),
     numeric_limits<float>::max()});
  vector<vector<double>> maxValues(
    this->size(),
    {numeric_limits<float>::lowest(),
     numeric_limits<float>::lowest(),
     numeric_limits<float>::lowest()});

  // We alternatively add faces, followed by vertices
  uint32_t *fp = (uint32_t*)buffer.data.data();
  float *vp = (float*)fp;

  // Now fill in the buffer. For each mesh we list its
  // faces, followed by its vertices
  for (int mesh_idx=0; mesh_idx<(int)this->size(); mesh_idx++)
  {
    size_t byte_offset = (uint8_t*)vp - (uint8_t*)buffer.data.data();

    const Mesh* mesh = &(*this)[mesh_idx];
    const auto& vertices = mesh->vertices; // shortcut
    int num_vertices = vertices.size();
    if (num_vertices == 0)
        continue;
    int num_faces = num_vertices/3;

    // Make use of the fact that the vertices are ordered
    fp = (uint32_t *) vp;
    for (int i=0; i<num_faces*3; i++)
      *fp++ = i;
  
    // And now the vertices
    vp = (float*)fp;
  
    for (auto& v : vertices)
    {
      float *pvp = vp; // Keep for min and max
  
      // Swap y and z
      *vp++ = zero_small_val(v[0]* export_scale);
      *vp++ = zero_small_val(v[2] * export_scale); 
      *vp++ = zero_small_val(-v[1] * export_scale);
      for (int i=0; i<3; i++)
      {
        float c = zero_small_val(*pvp++);

        if (c < minValues[mesh_idx][i])
          minValues[mesh_idx][i] = c;
        if (c > maxValues[mesh_idx][i])
          maxValues[mesh_idx][i] = c;
      }
    }

    // The indices
    tinygltf::BufferView bufferView;
    bufferView.buffer = 0;
    bufferView.byteOffset=byte_offset;
    bufferView.byteLength=num_faces*3*sizeof(uint32_t);
    bufferView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
    m.bufferViews.push_back(bufferView);

    // And the faces
    bufferView.buffer = 0;
    bufferView.byteOffset=byte_offset +bufferView.byteLength;
    bufferView.byteLength=num_vertices * 3 * sizeof(float);
    bufferView.target = TINYGLTF_TARGET_ARRAY_BUFFER;
    m.bufferViews.push_back(bufferView);

    
    // Describe the layout of bufferView1, the indices of the vertices
    tinygltf::Accessor accessor;
    accessor.bufferView = mesh_idx*2;
    accessor.byteOffset = 0;
    accessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    accessor.count = num_faces*3;
    accessor.type = TINYGLTF_TYPE_SCALAR;
    accessor.maxValues.push_back(num_vertices-1);
    accessor.minValues.push_back(0);
    m.accessors.push_back(accessor);
  
    // Describe the layout of bufferView2, the vertices themself
    accessor.bufferView = mesh_idx*2+1;
    accessor.byteOffset = 0;
    accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    accessor.count = num_vertices;
    accessor.type = TINYGLTF_TYPE_VEC3;
    accessor.maxValues = maxValues[mesh_idx];
    accessor.minValues = minValues[mesh_idx];
    m.accessors.push_back(accessor);

    // Build the mesh primitive and add it to the mesh
    primitive.indices = 2*mesh_idx;                 // The index of the accessor for the vertex indices
    primitive.attributes["POSITION"] = 2*mesh_idx+1;  // The index of the accessor for positions
    primitive.material = mesh_idx; 
    primitive.mode = TINYGLTF_MODE_TRIANGLES;
    tmesh.primitives = { primitive };
    m.meshes.push_back(tmesh);

    // Create a simple material
    tinygltf::Material mat;
    mat.pbrMetallicRoughness.baseColorFactor = {mesh->color.r,
                                                mesh->color.g,
                                                mesh->color.b,
                                                1.0f};
    mat.pbrMetallicRoughness.roughnessFactor = 0.5;

    fmt::print("Adding layer {} with color {},{},{}\n",
               m.materials.size(),
               mesh->color.r,
               mesh->color.g,
               mesh->color.b);

    mat.doubleSided = false;
    m.materials.push_back(mat);
  }

  // Build the mesh primitive and add it to the mesh
  primitive.indices = 0;                 // The index of the accessor for the vertex indices
  primitive.attributes["POSITION"] = 1;  // The index of the accessor for positions
  primitive.material = 0;
  primitive.mode = TINYGLTF_MODE_TRIANGLES;
  //  tmesh.primitives.push_back(primitive);

  // Build the nodes. First node is a reference to the children,
  // followed by the meshes as children
  vector<int> children;
  for (size_t i=0; i<size(); i++)
  {
    if ((&(*this)[i])->vertices.size() == 0)
      continue; // skip empty meshes
    children.push_back(i+1); // +1 since the children follow this node
  }
    
  node.children = children;
  node.name = fs::path(filename).filename().string();
  m.nodes.push_back(node);
  for (size_t i=0; i<size(); i++)
  {
    if ((&(*this)[i])->vertices.size() == 0)
      continue; // skip empty meshes

    node = tinygltf::Node();
    node.mesh = i;
    node.name = fmt::format("Layer {}", i);
    m.nodes.push_back(node);
  }

  scene.nodes.push_back(0); // Default scene

  // Define the asset. The version is required
  asset.version = "2.0";
  asset.generator = "tinygltf";

  // Now all that remains is to tie back all the loose objects into the
  // our single model.
  m.scenes.push_back(scene);
  m.buffers.push_back(buffer);
  m.asset = asset;

  // Save it to a file
  tinygltf::TinyGLTF gltf;
  gltf.WriteGltfSceneToFile(&m, filename.c_str(),
                           true, // embedImages
                           true, // embedBuffers
                           true, // pretty print
                           true); // write binary

}
