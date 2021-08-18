#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <font.h>
const GLuint WIDTH = 800, HEIGHT = 600;

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  

  GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "test", nullptr,
                                        nullptr);  // Windowed
  glfwMakeContextCurrent(window);
  gladLoadGL();
  
  glViewport(0, 0, WIDTH, HEIGHT);
  FontRenderer fr;
  fr.loadFont("../../arial.ttf", "arial");
  fr.loadFont("../../msyh.ttc", "msyh", true);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(window, 1);
    }
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    float time = glfwGetTime();
    float x = 25 + 100 * (1 + sin(time));
    float y = 25 + 150 * (1 + cos(time));
    float height = 48 + 18 * sin(time);
    float r = 0.5f + sin(time * 4 / 3) / 2;
    float g = 0.5f + sin(time * 4 / 5) / 2;
    float b = 0.5f + sin(time * 4 / 7) / 2;
    fr.draw("msyh", "This is 测试", x / WIDTH * 2 - 1, y / HEIGHT * 2 - 1, height, r, g, b);
    fr.draw("msyh", "特殊符号λγΠ", 0.35f, 0.9f, 24, 0.3f, 0.7f, 0.9f);
    fr.draw("arial", "another font test: arial", -0.5f, 2.0f / 3.0f, 32);
    glfwSwapBuffers(window);
  }
  glfwTerminate();
  return 0;
}