#ifndef PIXEL_GUI_WINDOW_H_
#define PIXEL_GUI_WINDOW_H_
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

struct WindowConfig {
  int width = 1920;
  int height = 1080;
  std::string title;
  bool centered = true;
  int major = 4;
  int minor = 3;
  bool resizable = false;
  bool visible = false;
  int xPos = -1;
  int yPos = -1;
  bool showfps = true;
  int swapInterval = 0;
};

class BaseWindow {
 public:
  BaseWindow(const WindowConfig &);

  void setWindowPos(int x, int y);
  std::pair<int, int> getWindowPos() const;
  GLFWmonitor *getBestMonitor() const;
  bool windowShouldClose() const;
  void setWindowShouldClose(int);
  void swapBuffers();
  void pollEvents();
  void showfps();
  GLFWwindow *getGlfwWindow() const { return window; }
  virtual void init();
  virtual void release();
  virtual void update();
  virtual void render();
  virtual ~BaseWindow();

  int getKey(int) const;
  double getDeltaTime() const;

 private:
  GLFWwindow *window;
  WindowConfig config;
};

void runProgram(BaseWindow &);
#endif