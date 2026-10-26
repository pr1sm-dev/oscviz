#pragma once

#include <GLFW/glfw3.h>
#include <string>

namespace window {
GLFWwindow *initializeGlfwWindow(int width, int height,
                                 const std::string &name);
void glfwErrorCallback(int code, const char *description);
void glfwFramebufferSizeCallback(GLFWwindow *window, int width, int height);
} // namespace window
