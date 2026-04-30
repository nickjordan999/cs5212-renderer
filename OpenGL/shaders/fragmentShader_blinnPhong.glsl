#version 330 core

layout(location=0) out vec4 fragmentColor;

uniform vec3 diffuseComponent;
uniform vec3 specularComponent;
uniform vec3 ambientComponent;
uniform float shininess;
uniform vec3 cameraPosWorld;

in vec4 normal;
in vec4 lightDir;
in vec4 worldPos;

void main(void)
{
  vec3 N = normalize(normal.xyz);
  vec3 L = normalize(lightDir.xyz);
  vec3 V = normalize(cameraPosWorld - worldPos.xyz);
  vec3 H = normalize(L + V);

  float diff = max(dot(N, L), 0.0);
  float spec = (diff > 0.0) ? pow(max(dot(N, H), 0.0), shininess) : 0.0;

  vec3 color = ambientComponent
             + diffuseComponent * diff
             + specularComponent * spec;

  fragmentColor = vec4(color, 1.0);
}
