#version 330 core

layout(location=0) out vec4 fragmentColor;

uniform vec3 diffuseComponent;

void main(void)
{
  fragmentColor = vec4(diffuseComponent, 1.0);
}
