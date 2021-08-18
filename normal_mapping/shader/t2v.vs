#version 430 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoords;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;

uniform mat4 model, view, proj;
uniform vec3 lightPos;

out vec3 varyingPos;
out vec2 varyingTexCoords;
out vec3 varyingNormal;
out vec3 varyingTangent;
out vec3 varyingLightDir;

void main() {
  mat4 mvMat = view * model;
  gl_Position = proj * mvMat * vec4(pos, 1.0);
  varyingPos = vec3(mvMat * vec4(pos, 1.0));
  varyingTexCoords = texCoords;
  mat4 normMat = transpose(inverse(mvMat));
  varyingNormal = vec3(normMat * vec4(normal, 1.0));
  varyingTangent = normalize(tangent - dot(tangent, normal) * normal);
  varyingTangent = vec3(normMat * vec4(varyingTangent, 1.0));
  varyingLightDir = vec3(view * vec4(lightPos, 1.0)) - varyingPos;
}