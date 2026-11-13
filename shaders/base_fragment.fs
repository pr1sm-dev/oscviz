#version 330 core 
uniform float time;

out vec4 FragColor;
in float depth;

void main() {
  float brightness = clamp((depth - (-1.0f) / (1.0f)), 0.0f, 1.0f);

  vec3 rgb = vec3(abs(sin(time/4.0f)),abs(sin(time/4.0f + 2.09)), abs(sin(time/4.0f + 4.19)));
  //vec3 rgb = vec3(0.0f, 0.58f, 1.0f);
  FragColor = vec4(rgb*brightness, 1.0f);
}
