#version 330 core

layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

void main(void)
{
  gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);
}
