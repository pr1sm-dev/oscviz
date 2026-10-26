#include <glad/glad.h>
#include <iostream>

#include <GLFW/glfw3.h>

#include "graphics/shader.hpp"
#include "window/window.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

int main() {
  if (!glfwInit()) {
    std::cout << "ERROR: Could not initialize GLFW" << std::endl;
    return -1;
  }
  GLFWwindow *window = window::initializeGlfwWindow(1920, 1080, "oscviz");
  if (!window) {
    std::cout << "ERROR: Could not create a GLFW window" << std::endl;
  }

  Shader s("shaders/base_vertex.vs", "shaders/base_fragment.fs");

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init();

  float vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f};

  GLuint VBO, VAO;
  glGenBuffers(1, &VBO);
  glGenVertexArrays(1, &VAO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void *)0);
  glEnableVertexAttribArray(0);

  while (!glfwWindowShouldClose(window)) {
    // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();

    glClear(GL_COLOR_BUFFER_BIT);

    float time = glfwGetTime();
    s.Use();
    s.SetFloat("time", time);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwTerminate();
  return -1;
}
