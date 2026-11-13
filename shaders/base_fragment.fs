#version 330 core 
uniform float time;

out vec4 FragColor;
in float depth;
in float b;
noperspective in vec3 vBarycentric;

void main() {
  float brightness = clamp(3*b + 0.50f, 0.0f, 1.0f);
  vec4 rgb = vec4(abs(sin(time/4.0f)),abs(sin(time/4.0f + 2.09)), abs(sin(time/4.0f + 4.19)), 1.0f);

  vec3 vDBaryCoordX = dFdx(vBarycentric);
  vec3 vDBaryCoordY = dFdy(vBarycentric);

  vec3 vDBaryCoord = sqrt(vDBaryCoordX*vDBaryCoordX + vDBaryCoordY * vDBaryCoordY);
  float thickness = 2.0;

  vec3 vRemap = smoothstep(vec3(0.0,0.0,0.0), vDBaryCoord * thickness, vBarycentric);

  float wireframe = min(vRemap.x, min(vRemap.y, vRemap.z));

  //vec3 rgb = vec3(0.0f, 0.58f, 1.0f);
  vec4 bg = vec4(0.0f,0.0f,0.0f,1.0f);
  //vec4 bg = vec4(0.094f, 0.0f, 0.188f, 1.0f);

  FragColor = mix(brightness*rgb, bg, wireframe);
}
