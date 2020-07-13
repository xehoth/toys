#include "Window.h"
#include "Shader.h"
#include <iostream>

union Color {
  struct {
    float r;
    float g;
    float b;
  };
  float a[3];

  float &operator[](int i) { return a[i]; }
  float operator[](int i) const { return a[i]; }
};

std::ostream &operator<<(std::ostream &os, const Color &c) {
  os << '(' << c.r << ", " << c.g << ", " << c.b << ')';
  return os;
}

const int WIDTH = 1920;
const int HEIGHT = 1080;

struct Pixel {
  float x, y;
  Color c;
} screen[WIDTH * HEIGHT];

std::ostream &operator<<(std::ostream &os, const Pixel &p) {
  os << '(' << p.x << ", " << p.y << "), " << p.c;
  return os;
}

inline void setPixel(int x, int y, Color c) {
  auto &p = screen[y * WIDTH + x];
  p.x = (x - WIDTH * 0.5f) / WIDTH * 2.0f;
  p.y = (y - HEIGHT * 0.5f) / HEIGHT * 2.0f;
  c.r /= 256.0f;
  c.g /= 256.0f;
  c.b /= 256.0f;
  p.c = c;
}

inline void doRender();

class Main : public BaseWindow {
  using BaseWindow::BaseWindow;

  GLuint vao, vbo;
  ShaderProgram pg;

  void init() override {
    BaseWindow::init();
    pg.init(VertexShader("main.vs"), FragmentShader("main.fs"));
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screen), screen, GL_STREAM_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Pixel),
                          reinterpret_cast<void *>(0));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Pixel),
                          reinterpret_cast<void *>(offsetof(Pixel, c)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    pg.use();
  }

  void update() override {
    BaseWindow::update();
    if (getKey(GLFW_KEY_ESCAPE) == GLFW_PRESS) setWindowShouldClose(GL_TRUE);
    doRender();
    glBufferData(GL_ARRAY_BUFFER, sizeof(screen), screen, GL_STREAM_DRAW);
  }

  void render() override {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_POINTS, 0, WIDTH * HEIGHT);
  }
};

int main() {
  WindowConfig config;
  config.width = WIDTH;
  config.height = HEIGHT;
  // config.swapInterval = 1;
  Main main(config);
  runProgram(main);
}

inline void doRender() {
#pragma omp parallel for
  for (int i = 0; i < WIDTH; ++i) {
    for (int j = 0; j < HEIGHT; ++j) {
      setPixel(i, j,
               {rand() % 256 * 1.0f, rand() % 256 * 1.0f, rand() % 256 * 1.0f});
    }
  }
}