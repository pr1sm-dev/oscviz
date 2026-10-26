#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <string>

class Shader {
public:
  Shader(const std::string &vertexShaderPath,
         const std::string &fragmentShaderPath);

  GLuint GetProgramId() { return programId_; }

  void Use() const;
  void SetBool(const std::string &name, bool value) const;
  void SetInt(const std::string &name, int value) const;
  void SetFloat(const std::string &name, float value) const;

private:
  GLuint programId_ = -1;
};
