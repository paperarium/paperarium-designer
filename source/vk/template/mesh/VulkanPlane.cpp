#include "mesh/VulkanPlane.h"

namespace VulkanEngine {

VulkanPlane::~VulkanPlane() noexcept {}

void VulkanPlane::generateVertex() {
  // build 6 vertices for two tris of plane
  std::vector<VertexTexVec4> vertices = {
      {{m_a, -0.0f, m_b}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
      {{-m_a, -0.0f, -m_b}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
      {{-m_a, -0.0f, m_b}, {0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
      {{m_a, -0.0f, m_b}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
      {{m_a, -0.0f, -m_b}, {1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
      {{-m_a, -0.0f, -m_b}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
  };
  // if transform is set, move the plane
  if (m_posOffset != glm::vec3(0.f)) staticMove(vertices);

  // setup the indices
  std::vector<uint32_t> indices(vertices.size());
  for (int i = 0; i < indices.size(); i++) indices[i] = i;
  m_indexCount = static_cast<uint32_t>(indices.size());

  // create buffers (in host memory, for simplicity)
  VK_CHECK_RESULT(m_context->vulkanDevice->createBuffer(
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      &m_vertexBuffer, vertices.size() * sizeof(VertexTexVec4),
      vertices.data()));
  VK_CHECK_RESULT(m_context->vulkanDevice->createBuffer(
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      &m_indexBuffer, indices.size() * sizeof(uint32_t), indices.data()));
}

}  // namespace VulkanEngine