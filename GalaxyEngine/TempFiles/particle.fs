#pragma version 460 core

layout(location = 0) in vec3 passColor;
layout(location = 1) in vec2 passUV;

layout(location = 0) out vec4 outColor; // First framebuffer attachment

void main(void)
{
  float lum = 1 - (passUV.x * passUV.x) - (passUV.y * passUV.y);
  outColor  = vec4(passColor, lum);
}