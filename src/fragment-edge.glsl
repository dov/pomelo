#version 410

smooth in vec4 vertexColor;
smooth in vec3 vBC;
out vec4 outputColor;

float edgeFactor(){
    vec3 d = fwidth(vBC);
    vec3 a3 = smoothstep(vec3(0.0), d*1.0, vBC);
    return min(min(a3.x, a3.y), a3.z);
}

void main()
{
  gl_FragColor = vec4(mix(vec3(0.0,0.5,0.0),
                          vertexColor.xyz,
                          edgeFactor()), 1.0);
}
