#ifndef _FONT_H_
#define _FONT_H_
#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>
#include <unordered_map>
#include <iostream>

static const std::string FONT_VERTEX_SHADER = R"(
#version 330 core

layout (location = 0) in vec4 vertex;
out vec2 varyingTexCoords;
uniform mat4 proj;

void main() {
  gl_Position = proj * vec4(vertex.xy, 0.0, 1.0);
  gl_Position.z = -1.0;
  varyingTexCoords = vertex.zw;
}
)";

static const std::string FONT_FRAGMENT_SHADER = R"(
#version 330 core
in vec2 varyingTexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main() {
  vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, varyingTexCoords).r);
  color = vec4(textColor, 1.0) * sampled;
}
)";

static const GLfloat FONT_DEFAULT_HEIGHT = 48;

// encoding helpers
// -----------------
#include <codecvt>

static inline std::wstring utf8ToUnicode(const std::string &str) {
  std::wstring ret;
  try {
    std::wstring_convert<std::codecvt_utf8<wchar_t> > wcv;
    ret = wcv.from_bytes(str);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  return ret;
}
// -----------------

class Font {
 public:
  Font() = default;
  Font(const std::string &fileName, std::string aliasFontName = "",
       bool unicode = false);
  bool loadChar(int c);

 private:
  struct Character {
    GLuint textureId;
    int sizeX, sizeY;     // size of glyph
    int bearingX, bearingY;  // offset from baseline to left/top of glyph
    GLuint advance;      // horizontal offset to advance to next glyph
  };
  std::string fileName;
  std::string fontName;
  std::unordered_map<wchar_t, Character> charBuffer;
  FT_Library ft;
  FT_Face face;
  friend class FontRenderer;
  void release() {
    if (face) {
      FT_Done_Face(face);
      face = nullptr;
    }
    if (ft) {
      FT_Done_FreeType(ft);
      ft = nullptr;
    }
  }
};

class FontRenderer {
 public:
  FontRenderer();
  void loadFont(const std::string &fileName, std::string aliasFontName = "",
                bool unicode = false);
  void loadFont(const Font &font);
  /**
   * draw text with textHeight and color in [fontName] font at (x, y)
   * 
   * If [fontName] is not loaded, use it as the path of the font file 
   * and [aliasFontName] can be used to give an alias to the font
   */
  void draw(const std::string &fontName, const std::string &text, 
            GLfloat x, GLfloat y, GLfloat textHeight = FONT_DEFAULT_HEIGHT,
            GLfloat r = 0, GLfloat g = 0, GLfloat b = 0,
            std::string aliasFontName = "", bool unicode = false);
  ~FontRenderer();
 private:
  void init();

  GLuint vao = 0;
  GLuint vbo = 0;
  GLuint shader = 0;
  std::unordered_map<std::string, Font> fontBuffer;
};

// --------------------------------------
// ------------- implement --------------
// --------------------------------------

Font::Font(const std::string &fileName, std::string aliasFontName,
           bool unicode) {
  if (aliasFontName.empty()) aliasFontName = fileName;
  this->fileName = fileName;
  this->fontName = aliasFontName;
  // All functions return a value different than 0 whenever an error occurred
  if (FT_Init_FreeType(&ft)) {
    std::cerr << "ERROR::FREETYPE: Could not init FreeType Library"
              << std::endl;
  }
  // Load font as face
  if (FT_New_Face(ft, fileName.c_str(), 0, &face)) {
    std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;
  }
  // change to unicode to support Chinese
  FT_Select_Charmap(face, FT_ENCODING_UNICODE);
  // Set size to load glyphs as
  FT_Set_Pixel_Sizes(face, 0, static_cast<FT_UInt>(FONT_DEFAULT_HEIGHT));

  for (int c = 0; c < 128; ++c) {
    loadChar(c);
  }
  if (!unicode) {
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    face = nullptr;
    ft = nullptr;
  }
}

bool Font::loadChar(int c) {
  if (charBuffer.count(c)) return true;
  if (!face || !ft) {
    std::cerr << "ERROR: ft/face is nullptr" << std::endl;
    return false;
  }
  // Disable byte-alignment restriction
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  // Load character glyph
  if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
    std::cerr << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
    return false;
  }
  // Generate texture
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width,
               face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
               face->glyph->bitmap.buffer);
  // Set texture options
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // Now store character for later use
  Character character = {texture,
                         static_cast<int>(face->glyph->bitmap.width),
                         static_cast<int>(face->glyph->bitmap.rows),
                         face->glyph->bitmap_left,
                         face->glyph->bitmap_top,
                         static_cast<GLuint>(face->glyph->advance.x)};
  charBuffer.emplace(c, character);
  glBindTexture(GL_TEXTURE_2D, 0);
  return true;
}

void FontRenderer::loadFont(const Font &font) {
  if (fontBuffer.count(font.fontName) || font.fontName.empty()) return;
  fontBuffer.emplace(font.fontName, font);
}

void FontRenderer::loadFont(const std::string &fileName,
                            std::string aliasFontName, bool unicode) {
  if (aliasFontName.empty()) {
    aliasFontName = fileName;
  }
  if (fontBuffer.count(aliasFontName)) return;
  fontBuffer.emplace(aliasFontName, Font(fileName, aliasFontName, unicode));
}

void FontRenderer::draw(const std::string &fontName,
                        const std::string &utf8Text, GLfloat x, GLfloat y,
                        GLfloat height, GLfloat r, GLfloat g, GLfloat b,
                        std::string aliasFontName, bool unicode) {
  GLfloat scale = height / FONT_DEFAULT_HEIGHT;
  init();
  loadFont(fontName, aliasFontName, unicode);
  if (aliasFontName.empty()) {
    aliasFontName = fontName;
  }
  std::wstring text = utf8ToUnicode(utf8Text);
  // Set OpenGL options
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glUseProgram(shader);
  glUniform3f(glGetUniformLocation(shader, "textColor"), r, g, b);
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(vao);

  glUseProgram(shader);
  GLfloat proj[4][4] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  };
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  GLfloat left = 0;
  GLfloat right = static_cast<GLfloat>(viewport[2]);
  GLfloat bottom = 0;
  GLfloat top = static_cast<GLfloat>(viewport[3]);
  proj[0][0] = static_cast<GLfloat>(2) / (right - left);
  proj[1][1] = static_cast<GLfloat>(2) / (top - bottom);
  proj[2][2] = - static_cast<GLfloat>(1);
  proj[3][0] = - (right + left) / (right - left);
  proj[3][1] = - (top + bottom) / (top - bottom);

  glUniformMatrix4fv(glGetUniformLocation(shader, "proj"), 1, GL_FALSE,
                     reinterpret_cast<const GLfloat *>(proj));
  // central origin point, range [-1.0, 1.0]
  x = (x + 1.0f) * 0.5f * right;
  y = (y + 1.0f) * 0.5f * top;

  auto &font = fontBuffer.find(aliasFontName)->second;;
  for (auto c : text) {
    if (!font.loadChar(c)) continue;
    auto it = font.charBuffer.find(c);
    const auto &ch = it->second;
    GLfloat xPos = x + ch.bearingX * scale;
    GLfloat yPos = y - (ch.sizeY - ch.bearingY) * scale;

    GLfloat w = ch.sizeX * scale;
    GLfloat h = ch.sizeY * scale;
    // Update vbo for each character
    GLfloat vertices[6][4] = {
        {xPos, yPos + h, 0.0, 0.0},    {xPos, yPos, 0.0, 1.0},
        {xPos + w, yPos, 1.0, 1.0},

        {xPos, yPos + h, 0.0, 0.0},    {xPos + w, yPos, 1.0, 1.0},
        {xPos + w, yPos + h, 1.0, 0.0}};
    // Render glyph texture over quad
    glBindTexture(GL_TEXTURE_2D, ch.textureId);
    // Update content of vbo memory
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(
        GL_ARRAY_BUFFER, 0, sizeof(vertices),
        vertices);  // Be sure to use glBufferSubData and not glBufferData
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Render quad
    glDrawArrays(GL_TRIANGLES, 0, 6);
    // Now advance cursors for next glyph (note that advance is number of 1/64
    // pixels)
    x += (ch.advance >> 6) *
         scale;  // Bitshift by 6 to get value in pixels (2^6 = 64 (divide
                 // amount of 1/64th pixels by 64 to get amount of pixels))
  }
}

void FontRenderer::init() {
  if (vao) return;
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  const GLchar *vsCode = FONT_VERTEX_SHADER.c_str();
  glShaderSource(vertexShader, 1, &vsCode, nullptr);
  glCompileShader(vertexShader);
  int success;
  char infoLog[1024];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, sizeof(infoLog), nullptr, infoLog);
    std::cerr << infoLog << std::endl;
  }

  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  const GLchar *fsCode = FONT_FRAGMENT_SHADER.c_str();
  glShaderSource(fragmentShader, 1, &fsCode, nullptr);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, sizeof(infoLog), nullptr, infoLog);
    std::cerr << infoLog << std::endl;
  }

  shader = glCreateProgram();
  glAttachShader(shader, vertexShader);
  glAttachShader(shader, fragmentShader);
  glLinkProgram(shader);
  glGetProgramiv(shader, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
    std::cerr << infoLog << std::endl;
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, nullptr,
               GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

FontRenderer::FontRenderer() { init(); }

FontRenderer::~FontRenderer() {
  for (auto it : fontBuffer) {
    it.second.release();
  }
}


/** example
// #include <glad/glad.h>
#include <GL/glew.h>
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
  // gladLoadGL();
  glewInit();
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
*/
#endif