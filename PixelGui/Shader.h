#ifndef PIXEL_GUI_SHADER_H_
#define PIXEL_GUI_SHADER_H_
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

template <GLenum type>
class Shader {
 public:
  Shader() = default;
  Shader(const std::string &path) { init(path); }

  void init(const std::string &path) {
    std::ifstream file;
    std::string code;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
      file.open(path);
      std::stringstream stream;
      stream << file.rdbuf();
      file.close();
      code = stream.str();
    } catch (std::ifstream::failure &e) {
      std::cerr << "[ERROR] Shader file error" << std::endl;
    }
    id = glCreateShader(type);
    const GLchar *source = code.c_str();
    glShaderSource(id, 1, &source, nullptr);
    glCompileShader(id);
    // check compile status
    GLchar infoLog[1024];
    GLint success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(id, 1024, nullptr, infoLog);
      std::cerr << "[ERROR] "
                << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment")
                << " shader compile error\n"
                << infoLog << std::endl;
    }
  }

  void release() {
    glDeleteShader(id);
    id = 0;
  }

  GLuint getId() const { return id; }

  ~Shader() { release(); }

 private:
  GLuint id = 0;
};

using VertexShader = Shader<GL_VERTEX_SHADER>;
using FragmentShader = Shader<GL_FRAGMENT_SHADER>;

class ShaderProgram {
 public:
  ShaderProgram() = default;
  ShaderProgram(const VertexShader &vertexShader,
                const FragmentShader &fragmentShader) {
    init(vertexShader, fragmentShader);
  }

  void init(const VertexShader &vertexShader,
            const FragmentShader &fragmentShader) {
    id = glCreateProgram();
    glAttachShader(id, vertexShader.getId());
    glAttachShader(id, fragmentShader.getId());
    glLinkProgram(id);
    // check link status
    GLchar infoLog[1024];
    GLint success;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(id, 1024, nullptr, infoLog);
      std::cerr << "[ERROR] Shader program link error\n"
                << infoLog << std::endl;
    }
  }

  void release() {
    glDeleteProgram(id);
    id = 0;
  }

  void use() const { glUseProgram(id); }

  GLint getUniformLocation(const std::string &name) const {
    return glGetUniformLocation(id, name.c_str());
  }

  void setBool(const std::string &name, GLboolean value) const {
    glUniform1i(getUniformLocation(name), static_cast<GLint>(value));
  }

  void setInt(const std::string &name, GLint value) const {
    glUniform1i(getUniformLocation(name), value);
  }

  void setFloat(const std::string &name, GLfloat value) const {
    glUniform1f(getUniformLocation(name), value);
  }

  void setVec3(const std::string &name, GLfloat x, GLfloat y, GLfloat z) const {
    glUniform3f(getUniformLocation(name), x, y, z);
  }

  void setVec3(const std::string &name, const glm::vec3 &value) const {
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
  }

  void setMat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE,
                       glm::value_ptr(mat));
  }

  ~ShaderProgram() { release(); }

 private:
  GLuint id = 0;
};
#endif