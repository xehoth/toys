#version 430 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;

uniform mat4 model, view, proj, normalMat;
uniform vec3 lightPos;

out vec2 varyingTexCoords;
out vec3 tangentLightPos, tangentPos;
out vec3 varyingNormal;

uniform int flag;

void main() {
  gl_Position = proj * view * model * vec4(pos, 1.0);
  vec3 varyingPos = vec3(view * model * vec4(pos, 1.0));
  varyingTexCoords = uv;
  vec3 N = normalize(normal);
  varyingNormal = vec3(normalMat * vec4(N, 1.0));
  vec3 T = normalize(tangent);
  T = normalize(T - dot(T, N) * N);
  vec3 B = cross(N, T);
  N = normalize(vec3(normalMat * vec4(N, 1.0)));
  T = normalize(vec3(normalMat * vec4(T, 1.0)));
  B = normalize(vec3(normalMat * vec4(B, 1.0)));
  mat3 TBN = mat3(T, B, N);
  if (flag == 1) TBN = mat3(1.0);
  tangentLightPos = TBN * lightPos;
  tangentPos = TBN * varyingPos;
}