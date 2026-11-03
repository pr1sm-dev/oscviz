#pragma once

#include "audio/engine.hpp"
#include "imgui/imgui.h"
#include <GLFW/glfw3.h>
#include <string>

namespace window {
struct MeshVisualizerState {
  float tilt;
  float amplitude;
  float frequency;
  float verticalOffset;
  float angularOffset;
};

class OscvizWindow {
public:
  OscvizWindow(int width, int height, const std::string &name);
  ~OscvizWindow();
  OscvizWindow(const OscvizWindow &w) = delete;
  OscvizWindow &operator=(const OscvizWindow &w) = delete;

  GLFWwindow *GetGLFWWindow() { return glfwWindow_; }

  void ShowMeshModeWindow();
  void ShowAudioConfigWindow();
  void ProcessKey(int key, int scancode, int action, int mode);
  void RenderUIWindows();

  MeshVisualizerState GetMeshVisualizerState() { return meshVisualizerState_; }

  static void GLFWKeyCallback(GLFWwindow *window, int key, int scancode,
                              int action, int mode);

private:
  audio::AudioEngine audioEngine;
  GLFWwindow *glfwWindow_;
  ImGuiIO *imguiIO_;

  char audioPath_[256] = {0};

  bool showUIElements_ = true;
  bool renderMeshUI_ = true;
  bool renderAudioConfigUI_ = true;
  MeshVisualizerState meshVisualizerState_;
};

GLFWwindow *initializeGlfwWindow(int width, int height,
                                 const std::string &name);
void glfwErrorCallback(int code, const char *description);
void glfwFramebufferSizeCallback(GLFWwindow *window, int width, int height);
} // namespace window
