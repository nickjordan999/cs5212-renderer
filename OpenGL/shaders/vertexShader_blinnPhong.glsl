#version 330 core

layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;

uniform vec4 lightPosWorld;

out vec4 normal;
out vec4 lightDir;
out vec4 worldPos;

void main(void)
{
  normal = normalize(normalMatrix * vec4(in_Normal, 0.0));

  worldPos = modelMatrix * vec4(in_Position, 1.0);
  lightDir = normalize(lightPosWorld - worldPos);

  gl_Position = projMatrix * viewMatrix * worldPos;
}
