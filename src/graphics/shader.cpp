#include "shader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

bool readShaderToString(const std::string &shaderPath,
                        std::string &shaderSource) {
  std::ifstream shaderStream(shaderPath);
  if (!shaderStream.is_open()) {
    return false;
  }

  std::stringstream shaderStrStream;
  shaderStrStream << shaderStream.rdbuf();

  shaderSource = shaderStrStream.str();

  return true;
}

bool checkShaderCompilation(GLuint shaderId) {
  int compiledSuccessfully = 0;
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compiledSuccessfully);

  if (!compiledSuccessfully) {
    int logLength = 0;
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);

    std::vector<char> infoLog(logLength);
    glGetShaderInfoLog(shaderId, logLength, &logLength, &infoLog[0]);

    std::string infoLogStr(infoLog.begin(), infoLog.end());

    std::cout << "ERROR: Shader id=" << shaderId
              << " failed to compile. Reason: " << infoLogStr;
  }

  return compiledSuccessfully;
}

bool checkProgramLinking(GLuint programId) {
  int linkedSuccessfully = 0;
  glGetProgramiv(programId, GL_LINK_STATUS, &linkedSuccessfully);

  if (!linkedSuccessfully) {
    int logLength = 0;
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength);

    std::vector<char> infoLog(logLength);
    glGetProgramInfoLog(programId, logLength, &logLength, &infoLog[0]);

    std::string infoLogStr(infoLog.begin(), infoLog.end());

    std::cout << "ERROR: Program id=" << programId
              << " failed to compile. Reason: \n"
              << infoLogStr;
  }

  return linkedSuccessfully;
}

Shader::Shader(const std::string &vertexShaderPath,
               const std::string &fragmentShaderPath) {
  std::string vertexShaderSource, fragmentShaderSource;

  if (!readShaderToString(vertexShaderPath, vertexShaderSource)) {
    std::cout << "ERROR: Could not open vertex shader " << vertexShaderPath
              << std::endl;
    return;
  } else if (!readShaderToString(fragmentShaderPath, fragmentShaderSource)) {
    std::cout << "ERROR: Could not open fragment shader " << fragmentShaderPath;
  }

  GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

  int compiled = 0;

  const char *vertexShaderSrc = vertexShaderSource.c_str();
  const char *fragmentShaderSrc = fragmentShaderSource.c_str();

  glShaderSource(vertexShaderId, 1, &vertexShaderSrc, 0);
  glShaderSource(fragmentShaderId, 1, &fragmentShaderSrc, 0);

  glCompileShader(vertexShaderId);
  glCompileShader(fragmentShaderId);

  if (!checkShaderCompilation(vertexShaderId)) {
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);
    return;
  }
  if (!checkShaderCompilation(fragmentShaderId)) {
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);
    return;
  }

  GLuint programId = glCreateProgram();
  glAttachShader(programId, vertexShaderId);
  glAttachShader(programId, fragmentShaderId);

  glLinkProgram(programId);

  if (!checkProgramLinking(programId)) {
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);
    glDeleteProgram(programId);
  }

  glDetachShader(programId, vertexShaderId);
  glDetachShader(programId, fragmentShaderId);

  programId_ = programId;
}

void Shader::SetBool(const std::string &name, bool value) const {
  GLint location = glGetUniformLocation(programId_, name.c_str());
  glUniform1i(location, (int)value);
}

void Shader::SetInt(const std::string &name, int value) const {
  GLint location = glGetUniformLocation(programId_, name.c_str());
  glUniform1i(location, value);
}

void Shader::SetFloat(const std::string &name, float value) const {
  GLint location = glGetUniformLocation(programId_, name.c_str());
  glUniform1f(location, value);
}

void Shader::Use() const {
  if (programId_ != -1) {
    glUseProgram(programId_);
  }
}
