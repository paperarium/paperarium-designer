#ifndef VULKAN_VERTEX_STRUCT_H
#define VULKAN_VERTEX_STRUCT_H

#include "render_common.h"

namespace VulkanEngine {

struct Vertex {
  float pos[3];
  float uv[2];
  float normal[3];
};

struct VertexUVW {
  float pos[3];
  float uv[3];
  float normal[3];
};

struct VertexTexVec4 {
  float pos[3];
  float uv[4];
  float normal[3];
};

}

#endif /*  VULKAN_VERTEX_STRUCT_H  */