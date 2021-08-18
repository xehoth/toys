#version 430 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoords;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;

uniform mat4 model, view, proj;
uniform vec3 lightPos, viewPos;

out vec3 varyingPos;
out vec2 varyingTexCoords;
out vec3 varyingLightPos;
out vec3 varyingViewPos;

void main() {
  gl_Position = proj * view * model * vec4(pos, 1.0);
  mat4 normMat = transpose(inverse(model));
  vec3 N = normalize(vec3(normMat * vec4(normal, 1.0)));
  vec3 T = normalize(vec3(normMat * vec4(tangent, 1.0)));
  T = normalize(T - dot(T, N) * N);
  vec3 B = cross(N, T);
  mat3 tbnInv = transpose(mat3(T, B, N));
  vec3 worldPos = vec3(model * vec4(pos, 1.0));
  varyingPos = tbnInv * worldPos;
  varyingTexCoords = texCoords;
  varyingLightPos = tbnInv * lightPos;
  varyingViewPos = tbnInv * viewPos;
}