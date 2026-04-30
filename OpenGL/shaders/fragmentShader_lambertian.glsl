#version 330 core

layout(location=0) out vec4 fragmentColor;

uniform vec3 diffuseComponent;

in vec4 normal;
in vec4 lightDir;

void main(void)
{
  float val = max(0.0, dot(normal, lightDir));
  vec3 diffuseShading = vec3( diffuseComponent.r * val, diffuseComponent.g * val, diffuseComponent.b * val );

  fragmentColor = vec4(diffuseShading, 1.0);
}
