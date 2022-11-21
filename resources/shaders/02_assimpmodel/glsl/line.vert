#version 450

layout(location = 0) in vec3 inPos;
layout(binding = 0) uniform UBO {
  mat4 projection;
  mat4 model;
  mat4 view;
  mat4 normal;
  vec4 lightpos;
}
ubo;

out gl_PerVertex { vec4 gl_Position; };

void main() {
  vec3 worldPos = vec3(ubo.model * vec4(inPos, 1.0));
  gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPos.xyz, 1.0);
}