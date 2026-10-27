#include <glad/glad.h>
#include <iostream>

#include <GLFW/glfw3.h>

#include "graphics/shader.hpp"
#include "graphics/sphere.hpp"
#include "window/window.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

int main() {
  window::OscvizWindow window(1920, 1080, "oscviz");

  Shader s("shaders/base_vertex.vs", "shaders/base_fragment.fs");

  Sphere sphere(1.0f, 1000, 100);

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

  glEnable(GL_DEPTH_TEST);

  while (!glfwWindowShouldClose(window.GetGLFWWindow())) {
    // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glfwPollEvents();

    float time = glfwGetTime();

    window.RenderUIWindows();

    window::MeshVisualizerState meshState = window.GetMeshVisualizerState();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    s.Use();
    s.SetFloat("time", time);
    s.SetFloat("tilt", meshState.tilt);
    s.SetFloat("amplitude", meshState.amplitude);
    s.SetFloat("frequency", meshState.frequency);
    s.SetFloat("verticalOffset", meshState.verticalOffset);
    s.SetFloat("angularOffset", meshState.angularOffset);

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
