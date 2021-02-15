#version 410

// A shader for an vbo supplying vertices, normals, and colors.
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 bary;

uniform mat4 projMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

smooth out vec3 tEye;
smooth out vec3 tNormal;
smooth out vec3 vBC;

void main() {
  vBC = bary;

  tEye = normalize( vec3( mvMatrix * vec4( position, 1.0 ) ) );
  tNormal = normalize( normalMatrix * normal );

  gl_Position = projMatrix * mvMatrix * vec4( position, 1. );
}

