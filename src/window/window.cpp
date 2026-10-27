#include "glad/glad.h"

#include "GLFW/glfw3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "window/window.hpp"

#include <iostream>

window::OscvizWindow::OscvizWindow(int width, int height,
                                   const std::string &name) {
  if (!glfwInit()) {
    std::cout << "ERROR: Could not initialize GLFW" << std::endl;
  }

  glfwWindow_ = window::initializeGlfwWindow(width, height, name);
  if (!glfwWindow_) {
    std::cout << "ERROR: Could not create a GLFW window" << std::endl;
    throw std::invalid_argument("Failed to create GLFW window");
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  imguiIO_ = &ImGui::GetIO();
  imguiIO_->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui_ImplGlfw_InitForOpenGL(glfwWindow_, true);
  ImGui_ImplOpenGL3_Init();

  glfwSetWindowUserPointer(glfwWindow_, this);
  glfwSetKeyCallback(glfwWindow_, window::OscvizWindow::GLFWKeyCallback);

  meshVisualizerState_.tilt = 80.0f;
  meshVisualizerState_.frequency = 16.0f;
  meshVisualizerState_.angularOffset = 0.0f;
  meshVisualizerState_.verticalOffset = 16.0f;
  meshVisualizerState_.amplitude = 0.02f;
}

window::OscvizWindow::~OscvizWindow() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwTerminate();
}

void window::OscvizWindow::ShowMeshModeWindow() {
  const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
  // ImGui::SetNextWindowPos(
  //     ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20),
  //     ImGuiCond_FirstUseEver);
  // ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_Always);

  if (!ImGui::Begin("Mesh Configuration", &showUIElements_, 0)) {
    ImGui::End();
    return;
  }

  ImGui::Text("Current tilt: %f", meshVisualizerState_.tilt);
  ImGui::SliderFloat("Tilt", &meshVisualizerState_.tilt, 0, 180);
  ImGui::SliderFloat("Amplitude", &meshVisualizerState_.amplitude, 0.0f, 0.3f);
  ImGui::SliderFloat("Frequency", &meshVisualizerState_.frequency, 0.0f,
                     100.0f);
  ImGui::SliderFloat("Vert. Offset", &meshVisualizerState_.verticalOffset, 0.0f,
                     100.0f);
  ImGui::SliderFloat("Angular. Offset", &meshVisualizerState_.angularOffset,
                     0.0f, 100.0f);

  ImGui::End();
}

void window::OscvizWindow::OscvizWindow::ProcessKey(int key, int scancode,
                                                    int action, int mode) {
  if (key == GLFW_KEY_H && action == GLFW_PRESS) {
    showUIElements_ = !showUIElements_;
  }
}

void window::OscvizWindow::OscvizWindow::RenderUIWindows() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  if (showUIElements_) {
    ImGui::ShowDemoWindow();
    ShowMeshModeWindow();
  }
}

void window::OscvizWindow::OscvizWindow::GLFWKeyCallback(GLFWwindow *window,
                                                         int key, int scancode,
                                                         int action, int mode) {
  void *data = glfwGetWindowUserPointer(window);
  OscvizWindow *app = static_cast<window::OscvizWindow *>(data);

  if (app) {
    app->ProcessKey(key, scancode, action, mode);
  }
}

void window::glfwErrorCallback(int code, const char *description) {
  std::cout << "ERROR: GLFW error " << std::hex << code << ": " << description
            << std::endl;
}

void window::glfwFramebufferSizeCallback(GLFWwindow *window, int width,
                                         int height) {
  glViewport(0, 0, width, height);
}

GLFWwindow *window::initializeGlfwWindow(int width, int height,
                                         const std::string &name) {
  glfwSetErrorCallback(window::glfwErrorCallback);
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  glfwWindowHint(GLFW_SAMPLES, 4);

  // create the window
  GLFWwindow *window = glfwCreateWindow(width, height, name.data(), NULL, NULL);
  if (window) {
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      std::cout << "Failed to initalize GLAD!" << std::endl;
      return NULL;
    }

    glfwSetFramebufferSizeCallback(window, window::glfwFramebufferSizeCallback);
  }

  glEnable(GL_MULTISAMPLE);

  return window;
}
