#version 430 core

in vec2 varyingTexCoords;
in vec3 tangentLightPos, tangentPos;
in vec3 varyingNormal;
layout (binding = 0) uniform sampler2D diffuseMap;
layout (binding = 1) uniform sampler2D normalMap;
uniform int normalMapping;
out vec4 fragColor;
void main() {
  vec3 normal = normalize(varyingNormal);
  if (normalMapping == 1)
    normal = texture(normalMap, varyingTexCoords).rgb;
  normal = normalize(normal * 2.0 - 1.0);
  vec3 color = texture(diffuseMap, varyingTexCoords).rgb;
  vec3 ambient = 0.1 * color;
  vec3 lightDir = normalize(tangentLightPos - tangentPos);
  vec3 diffuse = max(dot(lightDir, normal), 0.0) * color;
  vec3 viewDir = normalize(-tangentPos);
  vec3 halfVec = normalize(lightDir + viewDir);
  vec3 specular = vec3(0.2) * pow(max(dot(normal, halfVec), 0.0), 32.0);
  fragColor = vec4(ambient + diffuse + specular, 1.0);
}