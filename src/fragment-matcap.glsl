#version 430

uniform sampler2D texMatcap;

smooth in vec3 tEye;
smooth in vec3 tNormal;
smooth in vec3 vBC;

out vec4 outputColor;

void main() {
  vec3 r = reflect( tEye, tNormal );
  float m = 2. * sqrt( pow( r.x, 2. ) + pow( r.y, 2. ) + pow( r.z + 1., 2. ) );
  vec2 vN = r.xy / m + .5;

  vec3 base = texture( texMatcap, vN ).rgb;

  outputColor = vec4( base, 1. );
}
