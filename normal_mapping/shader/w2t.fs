#version 430 core

layout (binding = 0) uniform sampler2D diffuseMap;
layout (binding = 1) uniform sampler2D normalMap;
uniform int normalMapping;

in vec3 varyingPos;
in vec2 varyingTexCoords;
in vec3 varyingLightPos;
in vec3 varyingViewPos;
out vec4 fragColor;
vec3 calcNormal();

void main() {
  vec3 N = calcNormal();
  vec3 V = normalize(varyingViewPos - varyingPos);
  vec3 L = normalize(varyingLightPos - varyingPos);
  vec3 H = normalize(L + V);
  vec3 color = texture(diffuseMap, varyingTexCoords).rgb;
  vec3 ambient = 0.1 * color;
  vec3 diffuse = max(dot(L, N), 0.0) * color;
  vec3 specular = vec3(0.2) * pow(max(dot(N, H), 0.0), 32.0);
  fragColor = vec4(ambient + diffuse + specular, 1.0);
}

vec3 calcNormal() {
  vec3 normal = vec3(0.0, 0.0, 1.0);
  if (normalMapping == 1) {
    normal = texture(normalMap, varyingTexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);
  }
  return normal;
}