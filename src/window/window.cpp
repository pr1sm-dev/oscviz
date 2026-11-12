#include "audio/engine.hpp"
#include "glad/glad.h"

#include "GLFW/glfw3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "window/window.hpp"

#include <cstdio>
#include <cstring>
#include <nfd.h>

#include <iostream>

window::OscvizWindow::OscvizWindow(int width, int height,
                                   const std::string &name)
    : audioEngine(&currentTime_) {
  if (!glfwInit()) {
    std::cout << "ERROR: Could not initialize GLFW" << std::endl;
  }

  glfwWindow_ = window::initializeGlfwWindow(width, height, name);
  if (!glfwWindow_) {
    std::cout << "ERROR: Could not create a GLFW window" << std::endl;

    glfwTerminate();

    throw std::invalid_argument("Failed to create GLFW window");
    return;
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

  if (!ImGui::Begin("Mesh Configuration", &renderMeshUI_, 0)) {
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

void window::OscvizWindow::OscvizWindow::ShowAudioConfigWindow() {
  if (!ImGui::Begin("Audio Configuration", &renderAudioConfigUI_, 0)) {
    ImGui::End();
    return;
  }

  int paVersion = audioEngine.GetPortAudioVersion();

  ImGui::Text("PortAudio Version: %d", paVersion);
  const audio::AudioDevice &currentlySelectedDevice =
      audioEngine.GetCurrentOutputDevice();

  const std::vector<audio::AudioDevice> &outputAudioDevices =
      audioEngine.GetOutputDevices();

  for (const audio::AudioDevice &device : outputAudioDevices) {
    ImGui::Text("%s : default=%d : inputchannels=%d : outputchannels = %d",
                device.name.c_str(), device.isDefault, device.inputChannels,
                device.outputChannels);
    ImGui::Text("\t lowinlatency=%f : highinlatency=%f", device.lowInputLatency,
                device.highInputLatency);
    ImGui::Text("\t lowoutlatency=%f : highoutlatency=%f",
                device.lowOutputLatency, device.highOutputLatency);
  }

  if (ImGui::BeginCombo("Output Device", currentlySelectedDevice.name.c_str(),
                        0)) {
    for (int i = 0; i < outputAudioDevices.size(); i++) {
      bool isSelected = outputAudioDevices[i].id == currentlySelectedDevice.id;
      if (ImGui::Selectable(outputAudioDevices[i].name.c_str(), isSelected)) {
        audioEngine.SetOutputDevice(outputAudioDevices[i].id);
      }

      if (isSelected)
        ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }

  ImGuiInputTextFlags audioPathFlags =
      ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_ElideLeft;
  ImGui::InputText("Path", sourceAudioPath_, sizeof(sourceAudioPath_),
                   audioPathFlags);

  if (ImGui::SmallButton("Load")) {
    NFD_Init();

    nfdu8char_t *sourceAudioOutPath;

    nfdresult_t result =
        NFD_OpenDialogU8_With(&sourceAudioOutPath, &SourceAudioDialogArgs);
    if (result == NFD_OKAY) {
      std::snprintf(sourceAudioPath_, sizeof(sourceAudioPath_), "%s",
                    sourceAudioOutPath);
      NFD_FreePathU8(sourceAudioOutPath);

      audioEngine.LoadSourceAudioFile(sourceAudioPath_);
    }

    NFD_Quit();
  }

  switch (audioEngine.GetAudioFileStreamStatus()) {
  case audio::AudioFileStreamStatus::NoStream:
    ImGui::Text("No stream!");
    break;
  case audio::AudioFileStreamStatus::Initializing:
    ImGui::Text("Initializing..");
    break;
  case audio::AudioFileStreamStatus::Error:
    ImGui::Text("Error!");
    ImGui::Text("Error: %d", audioEngine.GetAudioFileStreamError());
    break;
  case audio::AudioFileStreamStatus::Done:
    ImGui::Text("Done!");
    break;
  case audio::AudioFileStreamStatus::Paused:
    ImGui::Text("Paused");
    break;
  case audio::AudioFileStreamStatus::Ready:
    audio::AudioFileStreamInfo streamInfo =
        audioEngine.GetAudioFileStreamInfo();
    ImGui::Text("Output channels: %d | Size of file: %ld frames | Length: "
                "%.2fms | Sample rate: %ld",
                streamInfo.channels, streamInfo.sizeInFrames,
                streamInfo.lengthMs, streamInfo.sampleRate);
    ImGui::Text("Current RMS L: %.2f | Current RMS R: %.2f", curLRms_,
                curRRms_);
    ImGui::Text("Current RMS: %.2f", curRms_);

    auto freqDomain = audioEngine.GetPlaybackFrequenciesDB();

    for (int i = 0; i < WINDOW_SIZE / 2; i++) {
      float amp = std::max(-40.0f, freqDomain[i]);
      ImGui::Text("%.1f - %.2f", amp, (float(i) * SAMPLE_RATE / WINDOW_SIZE));
    }

    break;
  }

  ImGui::End();
}

void window::OscvizWindow::ProcessAudio() {
  audioEngine.Process(glfwGetTime());
}

void window::OscvizWindow::OscvizWindow::RenderUIWindows() {
  double time = glfwGetTime();
  currentTime_.store(time);

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  if (showUIElements_) {
    ImGui::ShowDemoWindow();
    ShowMeshModeWindow();
    ShowAudioConfigWindow();
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
