#pragma version 460 core

layout(location = 0) out vec3 passColor;
layout(location = 1) out vec2 passUV;

struct InstanceData
{
  vec2 pos;
  vec3 color;
};

layout(location = 0) uniform mat4 projectionMatrix;
layout(std430, binding = 0) buffer instanceDataBuffer
{
  InstanceData instanceData[];
};

vec2 vertices[6] =
{
  vec2(-1.0f, -1.0f),
  vec2( 1.0f, -1.0f),
  vec2( 1.0f,  1.0f),
  vec2( 1.0f,  1.0f),
  vec2(-1.0f,  1.0f),
  vec2(-1.0f, -1.0f)
};

layout(location = 1) uniform vec2 u_Resolution;

void main(void)
{
  int vertexIndex   = gl_VertexID % 6;
  int instanceID    = gl_VertexID / 6;
  InstanceData data = instanceData[instanceID];

  vec2 uv     = vertices[vertexIndex];
  gl_Position = (projectionMatrix * vec4(data.pos, 0.0f, 1.0f)) + uv / u_Resolution; // Maybe use the inverse resolution...
  passColor   = data.color;
  passUV      = uv; // In this case we need the range [-1, 1] even though UVs are generally [0, 1]
}