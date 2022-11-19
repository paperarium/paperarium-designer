#include "mesh/VulkanCube.h"

namespace VulkanEngine {

VulkanCube::~VulkanCube() noexcept {}

void VulkanCube::generateVertex() {
  std::vector<VertexTexVec4> vertices = {
      // face 1
      {{m_a, m_b, m_c}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
      {{-m_a, -m_b, m_c}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
      {{-m_a, m_b, m_c}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
      {{m_a, m_b, m_c}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
      {{m_a, -m_b, m_c}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
      {{-m_a, -m_b, m_c}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
      // face 2
      {{m_a, m_b, -m_c}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
      {{-m_a, m_b, -m_c}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
      {{-m_a, -m_b, -m_c}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
      {{m_a, m_b, -m_c}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
      {{-m_a, -m_b, -m_c}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
      {{m_a, -m_b, -m_c}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
      // face 3
      {{m_a, m_b, m_c}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
      {{m_a, -m_b, -m_c}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
      {{m_a, -m_b, m_c}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
      {{m_a, m_b, m_c}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
      {{m_a, m_b, -m_c}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
      {{m_a, -m_b, -m_c}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
      // face 4
      {{-m_a, m_b, m_c}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
      {{-m_a, -m_b, m_c}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
      {{-m_a, -m_b, -m_c}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
      {{-m_a, m_b, m_c}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
      {{-m_a, -m_b, -m_c}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
      {{-m_a, m_b, -m_c}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}},
      // face 5
      {{m_a, m_b, m_c}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
      {{-m_a, m_b, m_c}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
      {{-m_a, m_b, -m_c}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
      {{m_a, m_b, m_c}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
      {{-m_a, m_b, -m_c}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
      {{m_a, m_b, -m_c}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
      // face 6
      {{m_a, -m_b, m_c}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
      {{-m_a, -m_b, -m_c}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
      {{-m_a, -m_b, m_c}, {0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
      {{m_a, -m_b, m_c}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}},
      {{m_a, -m_b, -m_c}, {1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
      {{-m_a, -m_b, -m_c}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}},
  };

  // apply mesh object transformation
  if (m_posOffset != glm::vec3(0.f)) {
    staticMove(vertices);
  }

  // set up vertex indices
  std::vector<uint32_t> indices(vertices.size());
  for (int i = 0; i < indices.size(); i++) indices[i] = i;
  m_indexCount = static_cast<uint32_t>(indices.size());

  // create buffers
  // vertex buffer. for simplicity, we don't stage to the GPUs
  VK_CHECK_RESULT(m_context->vulkanDevice->createBuffer(
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      &m_vertexBuffer, vertices.size() * sizeof(VertexTexVec4),
      vertices.data()));
  // index buffer
  VK_CHECK_RESULT(m_context->vulkanDevice->createBuffer(
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      &m_indexBuffer, indices.size() * sizeof(uint32_t), indices.data()));
}

}  // namespace VulkanEngine
