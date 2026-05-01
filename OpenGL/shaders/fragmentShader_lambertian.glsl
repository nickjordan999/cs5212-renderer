#version 330 core

layout(location=0) out vec4 fragmentColor;

in vec3 worldPos;
in vec3 worldNormal;

uniform vec3 diffuseComponent;

const int MAX_LIGHTS = 4;
uniform int numLights;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightIntensities[MAX_LIGHTS];

void main(void)
{
  vec3 N = normalize(worldNormal);
  vec3 sum = vec3(0.0);
  for (int i = 0; i < numLights; ++i) {
    vec3 L = normalize(lightPositions[i] - worldPos);
    float d = max(dot(N, L), 0.0);
    sum += diffuseComponent * d * lightIntensities[i];
  }
  fragmentColor = vec4(sum, 1.0);
}
