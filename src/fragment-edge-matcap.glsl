#version 430

uniform sampler2D texMatcap;

smooth in vec3 tEye;
smooth in vec3 tNormal;
smooth in vec3 vBC;

out vec4 outputColor;

float edgeFactor(){
    vec3 d = fwidth(vBC);
    vec3 a3 = smoothstep(vec3(0.0), d*1.0, vBC);
    return min(min(a3.x, a3.y), a3.z);
}

void main() {
  vec3 r = reflect( tEye, tNormal );
  float m = 2. * sqrt( pow( r.x, 2. ) + pow( r.y, 2. ) + pow( r.z + 1., 2. ) );
  vec2 vN = r.xy / m + .5;

  vec3 base = texture( texMatcap, vN ).rgb;

  outputColor = vec4(mix(vec3(0.0,0.5,0.0),
                         base,
                         edgeFactor()), 1.0);

}
