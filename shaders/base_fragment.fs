#version 330 core 
uniform float time;

out vec4 FragColor;

void main() {
  FragColor = vec4(abs(cos(time/2.0f)),abs(sin(time/3.0f)), abs(sin(time/4.0f) + 3.14f/4.0f), 0.8f);
}
