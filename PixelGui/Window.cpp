#include "Window.h"
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <functional>

BaseWindow::BaseWindow(const WindowConfig &config) : config(config) {}

void BaseWindow::setWindowPos(int x, int y) {
  glfwSetWindowPos(window, x, y);
  config.xPos = x;
  config.yPos = y;
}

std::pair<int, int> BaseWindow::getWindowPos() const {
  int x, y;
  glfwGetWindowPos(window, &x, &y);
  return {x, y};
}

GLFWmonitor *BaseWindow::getBestMonitor() const {
  int monitorCount;
  GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

  if (!monitors) return nullptr;

  int windowX, windowY, windowWidth, windowHeight;
  glfwGetWindowSize(window, &windowWidth, &windowHeight);
  glfwGetWindowPos(window, &windowX, &windowY);

  GLFWmonitor *bestMonitor = nullptr;
  int bestArea = 0;

  for (int i = 0; i < monitorCount; ++i) {
    GLFWmonitor *monitor = monitors[i];

    int monitorX, monitorY;
    glfwGetMonitorPos(monitor, &monitorX, &monitorY);

    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    if (!mode) continue;

    int areaMinX = std::max(windowX, monitorX);
    int areaMinY = std::max(windowY, monitorY);

    int areaMaxX = std::min(windowX + windowWidth, monitorX + mode->width);
    int areaMaxY = std::min(windowY + windowHeight, monitorY + mode->height);

    int area = (areaMaxX - areaMinX) * (areaMaxY - areaMinY);

    if (area > bestArea) {
      bestArea = area;
      bestMonitor = monitor;
    }
  }

  return bestMonitor;
}

bool BaseWindow::windowShouldClose() const {
  return glfwWindowShouldClose(window);
}

void BaseWindow::setWindowShouldClose(int value) {
  glfwSetWindowShouldClose(window, value);
}

void BaseWindow::swapBuffers() { glfwSwapBuffers(window); }

void BaseWindow::pollEvents() { glfwPollEvents(); }

void BaseWindow::showfps() {
  static double lastTime;
  static int frames;
  double currentTime = glfwGetTime();
  double delta = currentTime - lastTime;
  ++frames;
  if (delta >= 1.0) {
    double fps = frames / delta;
    glfwSetWindowTitle(
        window, (config.title + " [" + std::to_string(fps) + " FPS]").c_str());
    frames = 0;
    lastTime = currentTime;
  }
}

void BaseWindow::init() {
  std::cerr << "[INFO] BaseWindow init begin" << std::endl;
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config.major);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, config.minor);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, config.resizable);
  glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
  glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  window = glfwCreateWindow(config.width, config.height, config.title.c_str(),
                            nullptr, nullptr);
  if (!window) {
    std::cerr << "[ERROR] Failed to create glfw window" << std::endl;
    exit(-1);
  }

  if (config.centered) {
    const GLFWvidmode *vidmode = glfwGetVideoMode(getBestMonitor());
    setWindowPos((vidmode->width - config.width) / 2,
                 (vidmode->height - config.height) / 2);
  } else {
    if (config.xPos != -1 && config.yPos != -1) {
      setWindowPos(config.xPos, config.yPos);
    } else {
      std::tie(this->config.xPos, this->config.yPos) = getWindowPos();
    }
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(config.swapInterval);
  glfwShowWindow(window);
  if (!gladLoadGL()) {
    std::cerr << "[ERROR] Failed to init glad" << std::endl;
    exit(-1);
  }
  std::cerr << "[INFO] BaseWindow init done" << std::endl;
}

BaseWindow::~BaseWindow() { glfwTerminate(); }

void BaseWindow::release() {}
void BaseWindow::update() {
  if (config.showfps) showfps();
}
void BaseWindow::render() {}

int BaseWindow::getKey(int key) const { return glfwGetKey(window, key); }

void runProgram(BaseWindow &window) {
  window.init();
  while (!window.windowShouldClose()) {
    window.update();
    window.render();
    window.swapBuffers();
    window.pollEvents();
  }
  window.release();
}
