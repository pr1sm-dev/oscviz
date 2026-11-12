#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <math.h>

#include "audio/engine.hpp"
#include "graphics/shader.hpp"
#include "graphics/sphere.hpp"
#include "window/window.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"

int main() {
  window::OscvizWindow window(1920, 1080, "oscviz");

  Shader s("shaders/base_vertex.vs", "shaders/base_fragment.fs");

  Sphere sphere(1.0f, 128, 128);

  GLuint VBO, VAO, EBO;
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  glGenVertexArrays(1, &VAO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * sphere.GetVertices().size(),
               &sphere.GetVertices()[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(int) * sphere.GetIndices().size(),
               &sphere.GetIndices()[0], GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void *)0);
  glEnableVertexAttribArray(0);

  GLuint frequencyTexture;
  glGenTextures(1, &frequencyTexture);
  glBindTexture(GL_TEXTURE_1D, frequencyTexture);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

  glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, WINDOW_SIZE, 0, GL_RED, GL_FLOAT,
               nullptr);

  glEnable(GL_DEPTH_TEST);

  s.Use();
  s.SetInt("frequencyTexture", 0);

  double curLRms = 0.0;
  double curRRms = 0.0;
  double curRms = 0.0;

  while (!glfwWindowShouldClose(window.GetGLFWWindow())) {
    // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glfwPollEvents();

    window.RenderUIWindows();

    double time = glfwGetTime();

    window.ProcessAudio();

    curLRms = window.audioEngine.GetCurrentLRms();
    curRRms = window.audioEngine.GetCurrentRRms();
    curRms = std::sqrt(0.5 * curLRms * curLRms + 0.5 * curRRms + curRRms);

    window.SetRMS(curRms, curLRms, curRRms);

    window::MeshVisualizerState meshState = window.GetMeshVisualizerState();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, frequencyTexture);
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, WINDOW_SIZE, GL_RED, GL_FLOAT,
                    window.audioEngine.GetPlaybackFrequenciesDB().data());

    s.Use();
    s.SetFloat("time", time);
    s.SetFloat("tilt", meshState.tilt);
    s.SetFloat("amplitude", meshState.amplitude);
    s.SetFloat("frequency", meshState.frequency);
    s.SetFloat("verticalOffset", meshState.verticalOffset);
    s.SetFloat("angularOffset", meshState.angularOffset);
    s.SetFloat("power", curRms);
    s.SetFloat("powerL", curLRms);
    s.SetFloat("powerR", curRRms);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, sphere.GetIndices().size(), GL_UNSIGNED_INT,
                   0);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window.GetGLFWWindow());
  }

  return -1;
}
