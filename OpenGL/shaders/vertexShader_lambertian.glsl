#version 330 core

layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;

out vec3 worldPos;
out vec3 worldNormal;

void main(void)
{
  vec4 wp = modelMatrix * vec4(in_Position, 1.0);
  worldPos = wp.xyz;
  worldNormal = normalize((normalMatrix * vec4(in_Normal, 0.0)).xyz);
  gl_Position = projMatrix * viewMatrix * wp;
}
