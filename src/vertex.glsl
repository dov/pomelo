#version 410

// With ideas from multi-light-shader.glsl . 

// A shader for an vbo supplying vertices, normals, and colors.
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 bary;

uniform mat4 projMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

uniform float shininess;
uniform float specular;
uniform float diffuse;
uniform float ambient;
  
smooth out vec4 vertexColor;
smooth out vec3 vBC;

void xxmain() {
  gl_Position = projMatrix * mvMatrix * vec4(position, 1.0);
  vertexColor = vec4(color, 1.0);
}

void main() {
  vBC = bary;
  vec3 vertNormal = normalMatrix * normal; // Restore this for smooth shading!
  //  vec3 vertNormal = normal;

  // How is this different from vertNormal
  vec3 N = normalize((mvMatrix * vec4(normal,0.0)).xyz);

  vec4 mvPosition = mvMatrix * vec4(position,1.0);
  gl_Position = projMatrix * mvPosition;
  
  vec3 lightPos = vec3(-1e4,2e4,1e4); // TBD make this a uniform(sic?)
  vec3 lightPos1 = vec3(1e4,0.5e4,1e4); // TBD make this a uniform(sic?)
  vec3 lightPoss[2];
  lightPoss[0] = lightPos;
  lightPoss[1] = lightPos1;

  vertexColor = vec4(0,0,0,0);
  for (int i=0; i<2; i++) {
    vec3 L = normalize(lightPoss[i] - gl_Position.xyz);
  
    //Set up eye vector
    vec3 E = -normalize(mvPosition.xyz);
  
    //Set up Blinn half vector
    vec3 H = normalize(L+E); 
  
    //Calculate specular coefficient
    float Ks = pow(max(dot(N, H),0.0), shininess)*0.6;  
  
    //Calculate diffuse coefficient
    float Kd = max(dot(L,N), 0.0)*0.6;
  
    L = normalize(lightPos1 - gl_Position.xyz);
  
    //Set up Blinn half vector
    H = normalize(L+E); 
  
    //Calculate specular coefficient
    Ks += pow(max(dot(N, H),0.0), shininess)*0.5;  
  
    //Calculate diffuse coefficient
    Kd += max(dot(L,N), 0.0)*0.5;
  
    float NL = (ambient           // ambient
                + diffuse * Kd    // diffuse
                + specular * Ks); // specular
  
    //  frag_color =  vec4(color.rgb * NL, color.w);
    vertexColor += 0.5*vec4(color * NL,1);
  }
}
