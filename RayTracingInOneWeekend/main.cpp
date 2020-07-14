#include "Window.h"
#include "Shader.h"
#include "Ray.h"
#include "Sphere.h"
#include "Camera.h"
#include "Material.h"
#include <iostream>
#include <chrono>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const int WIDTH = 1920;
const int HEIGHT = 1080;
// const int WIDTH = 1280;
// const int HEIGHT = 720;

struct Pixel {
  float x, y;
  Color3f c;
} screen[WIDTH * HEIGHT];

std::ostream &operator<<(std::ostream &os, const Pixel &p) {
  os << '(' << p.x << ", " << p.y << "), " << p.c;
  return os;
}

inline void setPixel(int x, int y, Color3f c) {
  auto &p = screen[y * WIDTH + x];
  p.x = (x - WIDTH * 0.5f) / WIDTH * 2.0f;
  p.y = (y - HEIGHT * 0.5f) / HEIGHT * 2.0f;
  p.c = c;
  p.c.r = clamp(p.c.r, 0, 1);
  p.c.g = clamp(p.c.g, 0, 1);
  p.c.b = clamp(p.c.b, 0, 1);
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(screen), screen, GL_STREAM_DRAW);
  }

  void render() override {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_POINTS, 0, WIDTH * HEIGHT);
  }
};

void writeImage() {
  std::vector<unsigned char> data(WIDTH * HEIGHT * 4);
  for (int i = 0; i < WIDTH * HEIGHT; ++i) {
    data[i * 4 + 0] = static_cast<int>(screen[i].c.r * 255.99);
    data[i * 4 + 1] = static_cast<int>(screen[i].c.g * 255.99);
    data[i * 4 + 2] = static_cast<int>(screen[i].c.b * 255.99);
    data[i * 4 + 3] = 255;
  }
  stbi_flip_vertically_on_write(true);
  stbi_write_png("output.png", WIDTH, HEIGHT, 4, data.data(), 0);
}

#include <thread>

void render();

int main() {
  WindowConfig config;
  config.width = WIDTH;
  config.height = HEIGHT;
  // config.swapInterval = 10;
  std::thread th(render);
  Main main(config);
  runProgram(main);
  th.join();
}

const int MAX_DEPTH = 50;

Color3f rayColor(const Ray &r, const Hitable &objs, int dep) {
  if (dep <= 0) return Color3f(0, 0, 0);
  HitRecord rec;
  if (objs.hit(r, 0.001, std::numeric_limits<float>::infinity(), rec)) {
    Ray scattered;
    Color3f attenuation;
    if (rec.material->scatter(r, rec, attenuation, scattered))
      return attenuation * rayColor(scattered, objs, dep - 1);
    return Color3f(0, 0, 0);
  }

  auto uDir = normalize(r.dir);
  float t = 0.5f * (uDir.y + 1);
  return (1 - t) * Vec3f(1.0f, 1.0f, 1.0f) + t * Vec3f(0.5f, 0.7f, 1.0f);
}

HitList randomScene() {
  HitList world;
  auto groundMaterial = std::make_shared<Lambertian>(Color3f(0.5, 0.5, 0.5));
  world.add(
      std::make_shared<Sphere>(Point3f(0, -1000, 0), 1000.0f, groundMaterial));
  for (int a = -11; a < 11; ++a) {
    for (int b = -11; b < 11; ++b) {
      float chooseMat = randomFloat();
      Point3f center(a + 0.9 * randomFloat(), 0.2, b + 0.9 * randomFloat());
      if ((center - Point3f(4, 0.2, 0)).norm() > 0.9) {
        std::shared_ptr<Material> sphereMaterial;
        if (chooseMat < 0.8) {
          // diffuse
          auto albedo = randomVec3f() * randomVec3f();
          sphereMaterial = std::make_shared<Lambertian>(albedo);
          world.add(std::make_shared<Sphere>(center, 0.2f, sphereMaterial));
        } else if (chooseMat < 0.95) {
          // metal
          auto albedo = randomVec3f(0.5, 1);
          auto fuzz = randomFloat(0, 0.5);
          sphereMaterial = std::make_shared<Metal>(albedo, fuzz);
          world.add(std::make_shared<Sphere>(center, 0.2f, sphereMaterial));
        } else {
          // glass
          sphereMaterial = std::make_shared<Dielectric>(1.5f);
          world.add(std::make_shared<Sphere>(center, 0.2f, sphereMaterial));
        }
      }
    }
  }
  auto material1 = std::make_shared<Dielectric>(1.5f);
  world.add(std::make_shared<Sphere>(Point3f(0, 1, 0), 1.0f, material1));

  auto material2 = std::make_shared<Lambertian>(Color3f(0.4, 0.2, 0.1));
  world.add(std::make_shared<Sphere>(Point3f(-4, 1, 0), 1.0f, material2));

  auto material3 = std::make_shared<Metal>(Color3f(0.7, 0.6, 0.5), 0.0f);
  world.add(std::make_shared<Sphere>(Point3f(4, 1, 0), 1.0f, material3));

  return world;
}

void render() {
  std::cerr << "thread start" << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  std::cerr << "render start" << std::endl;
  auto start = std::chrono::high_resolution_clock::now();
  Point3f lookfrom(13, 2, 3), lookat(0, 0, 0), up(0, 1, 0);
  const float aspectRatio = static_cast<float>(WIDTH) / HEIGHT;
  float disToFocus = 10.0;
  float aperture = 0.1;
  Camera cam(lookfrom, lookat, up, 20, aspectRatio, aperture, disToFocus);

  HitList world = randomScene();
  const int SPP = 64;
  std::cerr << "SPP = " << SPP << std::endl;
#pragma omp parallel for
  for (int i = 0; i < WIDTH; ++i) {
    for (int j = 0; j < HEIGHT; ++j) {
      Color3f pc(0, 0, 0);
      for (int s = 0; s < SPP; ++s) {
        float u = (i + randomFloat()) / WIDTH;
        float v = (j + randomFloat()) / HEIGHT;
        Ray r = cam.getRay(u, v);
        pc += rayColor(r, world, MAX_DEPTH);
      }
      pc /= SPP;
      pc.r = sqrt(pc.r);
      pc.g = sqrt(pc.g);
      pc.b = sqrt(pc.b);
      setPixel(i, j, pc);
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::cerr
      << "done, cost: "
      << std::chrono::duration_cast<std::chrono::seconds>(end - start).count()
      << "s" << std::endl;
  std::cerr << "write image" << std::endl;
  writeImage();
  std::cerr << "done" << std::endl;
}
