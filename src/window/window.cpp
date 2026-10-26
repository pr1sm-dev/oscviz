#include "glad/glad.h"

#include "GLFW/glfw3.h"
#include "window/window.hpp"

#include <iostream>

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
