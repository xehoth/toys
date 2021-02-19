#version 430 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoords;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;

uniform mat4 model, view, proj;

out vec3 varyingPos;
out vec2 varyingTexCoords;
out vec3 varyingNormal;
out vec3 varyingTangent;

void main() {
  gl_Position = proj * view * model * vec4(pos, 1.0);
  varyingPos = vec3(model * vec4(pos, 1.0));
  varyingTexCoords = texCoords;
  mat4 normMat = transpose(inverse(model));
  varyingNormal = vec3(normMat * vec4(normal, 1.0));
  varyingTangent = vec3(model * vec4(tangent, 1.0));
}