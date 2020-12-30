#version 130

smooth in vec4 vertexColor;

out vec4 outputColor;

void main() {
  outputColor = vertexColor;
  //outputColor = vec4(vec3(gl_FragCoord.z), 1.0);
}
