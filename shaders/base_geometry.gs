#version 330 core 
layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

noperspective out vec3 vBarycentric;

in VS_OUT {
    float brightness;
} gs_in[];

out float b;

void main() {
  for (int i = 0; i < 3; i++) {
    if (i == 0) vBarycentric = vec3(1.0f, 0.0f, 0.0f);
    else if (i == 1) vBarycentric = vec3(0.0f, 1.0f, 0.0f);
    else vBarycentric = vec3(0.0f, 0.0f, 1.0f);
    
    b = gs_in[i].brightness;

    gl_Position = gl_in[i].gl_Position;
    EmitVertex();
  }  

  EndPrimitive();
}
