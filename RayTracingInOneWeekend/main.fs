#version 430 core

in vec3 mColor;
out vec4 fragColor;

void main() {
  fragColor = vec4(mColor, 1.0);
}