#version 330 core

layout(location=0) out vec4 fragmentColor;

in vec3 worldPos;
in vec3 worldNormal;

// Pattern (matches Plane::Pattern in src/Plane.h)
uniform int pattern;       // 0 = SOLID, 1 = CHECKER, 2 = HEX
uniform vec3 color1;
uniform vec3 color2;
uniform float patternScale;

// Shading model selection.
//   0 = simple (flat color), 1 = normal vis, 2 = lambertian, 3 = blinn-phong
uniform int shadingMode;

uniform vec3 ambientComponent;
uniform vec3 specularComponent;
uniform float shininess;
uniform vec3 cameraPosWorld;

const int MAX_LIGHTS = 4;
uniform int numLights;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightIntensities[MAX_LIGHTS];

vec3 pickPatternColor(vec3 wp)
{
  if (pattern == 1) {
    int ix = int(floor(wp.x / patternScale));
    int iz = int(floor(wp.z / patternScale));
    int parity = ((ix + iz) % 2 + 2) % 2;
    return (parity != 0) ? color2 : color1;
  }
  if (pattern == 2) {
    float sqrt3 = sqrt(3.0);
    float fq =  (2.0 / 3.0)  * wp.x / patternScale;
    float fr =  (-1.0 / 3.0) * wp.x / patternScale + (sqrt3 / 3.0) * wp.z / patternScale;
    float fs = -fq - fr;
    int qi = int(floor(fq + 0.5));
    int ri = int(floor(fr + 0.5));
    int si = int(floor(fs + 0.5));
    float qd = abs(fq - float(qi));
    float rd = abs(fr - float(ri));
    float sd = abs(fs - float(si));
    if (qd > rd && qd > sd)      qi = -ri - si;
    else if (rd > sd)            ri = -qi - si;
    int parity = ((qi % 2) + 2) % 2;
    return (parity != 0) ? color2 : color1;
  }
  return color1;  // SOLID
}

void main(void)
{
  vec3 N = normalize(worldNormal);
  vec3 baseColor = pickPatternColor(worldPos);

  if (shadingMode == 1) {
    fragmentColor = vec4(N * 0.5 + 0.5, 1.0);
    return;
  }
  if (shadingMode == 0) {
    fragmentColor = vec4(baseColor, 1.0);
    return;
  }

  vec3 V = normalize(cameraPosWorld - worldPos);

  if (shadingMode == 2) {
    // Lambertian: only diffuse contribution from each light.
    vec3 sum = vec3(0.0);
    for (int i = 0; i < numLights; ++i) {
      vec3 L = normalize(lightPositions[i] - worldPos);
      float d = max(dot(N, L), 0.0);
      sum += baseColor * d * lightIntensities[i];
    }
    fragmentColor = vec4(sum, 1.0);
    return;
  }

  // Blinn-Phong (shadingMode == 3 or fallback)
  vec3 sum = ambientComponent;
  for (int i = 0; i < numLights; ++i) {
    vec3 L = normalize(lightPositions[i] - worldPos);
    float diff = max(dot(N, L), 0.0);
    vec3 H = normalize(L + V);
    float spec = (diff > 0.0) ? pow(max(dot(N, H), 0.0), shininess) : 0.0;
    sum += (baseColor * diff + specularComponent * spec) * lightIntensities[i];
  }
  fragmentColor = vec4(sum, 1.0);
}
