#version 330 core 
uniform float time;

out vec4 FragColor;
in float depth;

void main() {
  float brightness = clamp((depth - (-1.0f) / (1.0f)), 0.0f, 1.0f);

  vec3 rgb = vec3(abs(cos(time/2.0f)),abs(sin(time/3.0f)), abs(sin(time/4.0f) + 3.14f/4.0f));
  FragColor = vec4(rgb*brightness, 1.0f);
}
