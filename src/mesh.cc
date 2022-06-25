//======================================================================
//  mesh.h - A simple mesh interface for saving to STL and gltf.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Mon Oct 12 22:10:45 2020
//----------------------------------------------------------------------
#include "mesh.h"
#include <fstream>
#include <iostream>
#include "fmt/core.h"

// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tinygltf/tiny_gltf.h"

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

// 3D Mesh routines
void save_stl(std::shared_ptr<Mesh> mesh, const std::string& filename)
{
  // Write header
  const int header_size = 80;
  char header[header_size];
  std::fill_n(header, header_size, 0);
  ofstream fh(filename,ofstream::binary|ofstream::out);
  
  fh.write(header, header_size);

  // Make use of the fact that all the vertices are ordered in
  // groups of three composing the contained triangles.
  const  auto& vertices = mesh->vertices; // shortcut
  size_t size = (size_t)vertices.size()/3;
  cout << fmt::format("mesh.size={}\n", size);
  fh.write((const char*)&size, 4);

  uint16_t color=0;
  for(size_t tr_idx=0; tr_idx<size; tr_idx++)
    {
      float fzero {0};
      for (int i=0; i<3; i++)
        fh.write((const char*)&fzero,4);

      for (int i=0; i<3; i++)
        fh.write((char*)(&vertices[tr_idx*3+i][0]), 3*4);
      fh.write((char*)&color, 2);
    }
  fh.close();
}

void save_gltf(std::shared_ptr<Mesh> mesh, const std::string& filename)
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


  const  auto& vertices = mesh->vertices; // shortcut
  print("num_vertices={}\n", vertices.size());
  int num_vertices = vertices.size();
  int num_faces = num_vertices/3;
  buffer.data.resize(num_faces * 3 * sizeof(int32_t)
                     + num_vertices * 3 * sizeof(float));

  // First place the faces
  uint8_t *p = buffer.data.data();
  uint32_t *fp = (uint32_t*)p;

  // Make use of the fact that the vertices are ordered
  for (int i=0; i<num_faces*3; i++)
    *fp++ = i;

  // And now the vertices
  float *vp = (float*)fp;
  // Vertices
  vector<double> minValues = {numeric_limits<float>::max(),numeric_limits<float>::max(),numeric_limits<float>::max()};
  vector<double> maxValues = {numeric_limits<float>::lowest(),numeric_limits<float>::lowest(),numeric_limits<float>::lowest()};

  for (auto& v : vertices)
    {
      float *pvp = vp; // Keep for min and max

      // Swap y and z
      *vp++ = v[0];
      *vp++ = v[2]; 
      *vp++ = -v[1];
      for (int i=0; i<3; i++)
        {
          if (*pvp < minValues[i])
            minValues[i] = *pvp;
          if (*pvp > maxValues[i])
            maxValues[i] = *pvp;
          pvp++;
        }
    }
  
  // The indices
  bufferView1.buffer = 0;
  bufferView1.byteOffset=0;
  bufferView1.byteLength=num_faces*3*sizeof(uint32_t);
  bufferView1.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

  // And the faces
  bufferView2.buffer = 0;
  bufferView2.byteOffset=bufferView1.byteLength;
  bufferView2.byteLength=num_vertices * 3 * sizeof(float);
  bufferView2.target = TINYGLTF_TARGET_ARRAY_BUFFER;

  // Describe the layout of bufferView1, the indices of the vertices
  accessor1.bufferView = 0;
  accessor1.byteOffset = 0;
  accessor1.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
  accessor1.count = num_faces*3;
  accessor1.type = TINYGLTF_TYPE_SCALAR;
  accessor1.maxValues.push_back(num_vertices-1);
  accessor1.minValues.push_back(0);

  // Describe the layout of bufferView2, the vertices themself
  accessor2.bufferView = 1;
  accessor2.byteOffset = 0;
  accessor2.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
  accessor2.count = num_vertices;
  accessor2.type = TINYGLTF_TYPE_VEC3;
  accessor2.maxValues = maxValues;
  accessor2.minValues = minValues;

  // Build the mesh primitive and add it to the mesh
  primitive.indices = 0;                 // The index of the accessor for the vertex indices
  primitive.attributes["POSITION"] = 1;  // The index of the accessor for positions
  //  primitive.material = 0;
  primitive.mode = TINYGLTF_MODE_TRIANGLES;
  tmesh.primitives.push_back(primitive);

  // Other tie ups
  node.mesh = 0;
  scene.nodes.push_back(0); // Default scene

  // Define the asset. The version is required
  asset.version = "2.0";
  asset.generator = "tinygltf";

  // Now all that remains is to tie back all the loose objects into the
  // our single model.
  m.scenes.push_back(scene);
  m.meshes.push_back(tmesh);
  m.nodes.push_back(node);
  m.buffers.push_back(buffer);
  m.bufferViews.push_back(bufferView1);
  m.bufferViews.push_back(bufferView2);
  m.accessors.push_back(accessor1);
  m.accessors.push_back(accessor2);
  m.asset = asset;

#if 0
  // Create a simple material
  tinygltf::Material mat;
  mat.pbrMetallicRoughness.baseColorFactor = {1.0f, 0.9f, 0.9f, 1.0f};  
  mat.doubleSided = false;
  m.materials.push_back(mat);
#endif

  // Save it to a file
  tinygltf::TinyGLTF gltf;
  gltf.WriteGltfSceneToFile(&m, filename.c_str(),
                           true, // embedImages
                           true, // embedBuffers
                           true, // pretty print
                           true); // write binary

}
