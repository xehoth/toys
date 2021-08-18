#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

class Shader {
 public:
  Shader() = default;
  Shader(const std::string &vsPath, const std::string &fsPath) {
    init(vsPath, fsPath);
  }
  void init(const std::string &, const std::string &);
  void initWithCode(const std::string &, const std::string &);
  static std::string getCodeFromFile(const std::string &);
  void use() const;
  GLint get(const std::string &) const;
  void setInt(const std::string &, GLint) const;
  void setFloat(const std::string &, GLfloat) const;
  void setMat4(const std::string &, const glm::mat4 &) const;
  void setVec3(const std::string &name, const glm::vec3 &value) const;

 private:
  GLuint id = 0;
};

GLuint loadTexture(const std::string &);
void processInput(GLFWwindow *, Shader *&);
void setupQuad();
void render(float, Shader *);

const int SRC_WIDTH = 800 * 2;
const int SRC_HEIHGT = 600 * 2;
const float ASPECT = static_cast<float>(SRC_WIDTH) / SRC_HEIHGT;

struct Vertex {
  glm::vec3 pos;
  glm::vec2 uv;
  glm::vec3 normal;
  glm::vec3 tangent;
};

struct Triangle {
  Vertex a, b, c;

  void calcTangents() {
    glm::vec3 edge1 = b.pos - a.pos;
    glm::vec3 edge2 = c.pos - a.pos;
    float deltaU1 = b.uv.s - a.uv.s;
    float deltaV1 = b.uv.t - a.uv.t;
    float deltaU2 = c.uv.s - a.uv.s;
    float deltaV2 = c.uv.t - a.uv.t;
    float det = 1.0f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);
    glm::vec3 tangent(det * (deltaV2 * edge1.x - deltaV1 * edge2.x),
                      det * (deltaV2 * edge1.y - deltaV1 * edge2.y),
                      det * (deltaV2 * edge1.z - deltaV1 * edge2.z));
    // no need for bitangent, which is calculated in the shader
    tangent = glm::normalize(tangent);
    a.tangent = b.tangent = c.tangent = tangent;
  }
};

struct Quad {
  Triangle first;
  Triangle second;

  void calcTangents() {
    first.calcTangents();
    second.calcTangents();
  }
};

glm::vec3 lightPos(0.5f, 1.0f, 0.3f);
glm::vec3 cameraPos(0.0f, 0.0f, 3.0f);
Shader t2wShader, t2vShader, w2tShader;
int normalMapping = 1;
bool rotating = false;
GLuint diffuseMap, normalMap;

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  GLFWwindow *window =
      glfwCreateWindow(SRC_WIDTH, SRC_HEIHGT, "normal", nullptr, nullptr);
  if (!window) {
    std::cerr << "failed to create window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (!gladLoadGL()) {
    std::cerr << "failed to load glad" << std::endl;
    return -1;
  }
  glViewport(0, 0, SRC_WIDTH, SRC_HEIHGT);
  glEnable(GL_DEPTH_TEST);
  t2wShader.init("t2w.vs", "t2w.fs");
  t2vShader.init("t2v.vs", "t2v.fs");
  w2tShader.init("w2t.vs", "w2t.fs");
  Shader *shader = &t2wShader;
  diffuseMap = loadTexture("brickwall.jpg");
  normalMap = loadTexture("brickwall_normal.jpg");

  GLuint vao, vbo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  setupQuad();

  while (!glfwWindowShouldClose(window)) {
    processInput(window, shader);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render(glfwGetTime(), shader);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
}

void setupQuad() {
  glm::vec3 pos[4]{{-1.0f, 1.0f, 0.0f},
                   {-1.0f, -1.0f, 0.0f},
                   {1.0f, -1.0f, 0.0f},
                   {1.0f, 1.0f, 0.0f}};
  glm::vec2 uv[4]{{0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}};
  glm::vec3 normal(0.0f, 0.0f, 1.0f);
  Quad quad{{{pos[0], uv[0], normal, {}},
             {pos[1], uv[1], normal, {}},
             {pos[2], uv[2], normal, {}}},
            {{pos[0], uv[0], normal, {}},
             {pos[2], uv[2], normal, {}},
             {pos[3], uv[3], normal, {}}}};
  quad.calcTangents();
  glBufferData(GL_ARRAY_BUFFER, sizeof(Quad), &quad, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void *>(offsetof(Vertex, pos)));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void *>(offsetof(Vertex, uv)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void *>(offsetof(Vertex, normal)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void *>(offsetof(Vertex, tangent)));
  glEnableVertexAttribArray(3);
}

void processInput(GLFWwindow *window, Shader *&shader) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE))
    glfwSetWindowShouldClose(window, true);
  if (glfwGetKey(window, GLFW_KEY_Q)) shader = &t2wShader;
  if (glfwGetKey(window, GLFW_KEY_W)) shader = &t2vShader;
  if (glfwGetKey(window, GLFW_KEY_E)) shader = &w2tShader;
  if (glfwGetKey(window, GLFW_KEY_N)) normalMapping = 1;
  if (glfwGetKey(window, GLFW_KEY_M)) normalMapping = 0;
  if (glfwGetKey(window, GLFW_KEY_J)) rotating = true;
  if (glfwGetKey(window, GLFW_KEY_K)) rotating = false;
}

void render(float currentTime, Shader *shader) {
  static float rad = 0.0f, step = 0.001f;
  if (rotating) {
    if (rad > 0.75f) step = -step;
    if (rad < -0.75f) step = -step;
    rad += step;
  } else {
    rad = 0.0f;
  }
  shader->use();
  shader->setInt("normalMapping", normalMapping);
  glm::mat4 model(1.0f);
  model = glm::rotate(model, rad, glm::vec3(1.0f, 1.0f, 1.0f));
  glm::mat4 view(1.0f);
  view = glm::translate(view, -cameraPos);
  glm::mat4 proj = glm::perspective(glm::radians(45.0f), ASPECT, 0.1f, 100.0f);
  shader->setMat4("model", model);
  shader->setMat4("view", view);
  shader->setMat4("proj", proj);
  shader->setVec3("lightPos", lightPos);
  shader->setVec3("viewPos", cameraPos);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, diffuseMap);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, normalMap);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

GLuint loadTexture(const std::string &path) {
  GLuint id;
  glGenTextures(1, &id);
  int width, height, nrComponents;
  stbi_uc *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
  if (data) {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::cerr << "failed to load texture " << path << std::endl;
    stbi_image_free(data);
  }
  return id;
}

void Shader::init(const std::string &vs, const std::string &fs) {
  initWithCode(getCodeFromFile(vs), getCodeFromFile(fs));
}

void Shader::initWithCode(const std::string &vs, const std::string &fs) {
  GLuint vertexShader, fragmentShader;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  const GLchar *vsCode = vs.c_str();
  glShaderSource(vertexShader, 1, &vsCode, nullptr);
  glCompileShader(vertexShader);
  int success;
  char infoLog[1024];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, sizeof(infoLog), nullptr, infoLog);
    std::cerr << infoLog << std::endl;
  }

  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  const GLchar *fsCode = fs.c_str();
  glShaderSource(fragmentShader, 1, &fsCode, nullptr);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, sizeof(infoLog), nullptr, infoLog);
    std::cerr << infoLog << std::endl;
  }
  id = glCreateProgram();
  glAttachShader(id, vertexShader);
  glAttachShader(id, fragmentShader);
  glLinkProgram(id);
  glGetProgramiv(id, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(id, sizeof(infoLog), nullptr, infoLog);
    std::cerr << infoLog << std::endl;
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

std::string Shader::getCodeFromFile(const std::string &path) {
  std::string code;
  std::ifstream file;
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try {
    file.open(path);
    std::stringstream stream;
    stream << file.rdbuf();
    file.close();
    code = stream.str();
  } catch (std::ifstream::failure &e) {
    std::cerr << "File Error" << std::endl << e.what() << std::endl;
  }
  return code;
}

void Shader::use() const { glUseProgram(id); }
GLint Shader::get(const std::string &name) const {
  return glGetUniformLocation(id, name.c_str());
}
void Shader::setInt(const std::string &name, GLint value) const {
  glUniform1i(get(name), value);
}

void Shader::setFloat(const std::string &name, GLfloat value) const {
  glUniform1f(get(name), value);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &value) const {
  glUniformMatrix4fv(get(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const {
  glUniform3fv(get(name), 1, glm::value_ptr(value));
}
