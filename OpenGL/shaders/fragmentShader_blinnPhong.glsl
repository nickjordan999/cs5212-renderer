#version 330 core

layout(location=0) out vec4 fragmentColor;

in vec3 worldPos;
in vec3 worldNormal;

uniform vec3 diffuseComponent;
uniform vec3 specularComponent;
uniform vec3 ambientComponent;
uniform float shininess;
uniform vec3 cameraPosWorld;

const int MAX_LIGHTS = 4;
uniform int numLights;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightIntensities[MAX_LIGHTS];

void main(void)
{
  vec3 N = normalize(worldNormal);
  vec3 V = normalize(cameraPosWorld - worldPos);

  vec3 sum = ambientComponent;
  for (int i = 0; i < numLights; ++i) {
    vec3 L = normalize(lightPositions[i] - worldPos);
    float diff = max(dot(N, L), 0.0);
    vec3 H = normalize(L + V);
    float spec = (diff > 0.0) ? pow(max(dot(N, H), 0.0), shininess) : 0.0;
    sum += (diffuseComponent * diff + specularComponent * spec) * lightIntensities[i];
  }

  fragmentColor = vec4(sum, 1.0);
}
