#include "01_statictriangle/objects/Triangle.h"

namespace VulkanEngine {

Triangle::~Triangle() {}

void Triangle::generateVertex() {
  // define triangle vertices
  std::vector<VertexTexVec4> vertices = {
    {{ 0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.f}, {0.0f, 0.0f, 1.0f}},
    {{ 1.0f,  1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.f}, {0.0f, 0.0f, 1.0f}},
    {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.f}, {0.0f, 0.0f, 1.0f}},
  };

  // setup indices
  std::vector<uint32_t> indices = { 0, 1, 2 };
  m_indexCount = static_cast<uint32_t>(indices.size());

  // create the buffers
  // for sake of simplicity, we don't stage the vertex data to GPU memory

  // vertex buffer
  VK_CHECK_RESULT(m_context->vulkanDevice->createBuffer(
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    &m_vertexBuffer,
    vertices.size() * sizeof(VertexTexVec4),
    vertices.data()
  ));
  // index buffer
  VK_CHECK_RESULT(m_context->vulkanDevice->createBuffer(
    VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    &m_indexBuffer,
    indices.size() * sizeof(uint32_t),
    indices.data()
  ));
}

}