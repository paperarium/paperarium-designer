#include "mesh/MeshObject.h"

namespace VulkanEngine {

MeshObject::~MeshObject() {
  m_vertexBuffer.destroy();
  m_indexBuffer.destroy();
}

void MeshObject::prepare() {
  generateVertex();
}

void MeshObject::update() {
  updateVertex();
}

/**
 * @brief Adds this mesh object to the command buffer
 * 
 * @param cmdBuffer The command buffer to add the commands to
 * @param vulkanShader The shader whose pipeline we want to bind for this mesh
 */
void MeshObject::build(VkCommandBuffer &cmdBuffer, VulkanShader* vulkanShader) {
  VkDeviceSize offsets[1] = {0};
  if (vulkanShader->getPipeline()) {
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanShader->getPipeline());
  } else {
    LOGI("%s", "Pipeline null, bind failure.");
  }
  vkCmdBindVertexBuffers(cmdBuffer, VERTEX_BUFFER_BIND_ID, 1, &m_vertexBuffer.buffer, offsets);
  vkCmdBindIndexBuffer(cmdBuffer, m_indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
  vkCmdDrawIndexed(cmdBuffer, m_indexCount, 1, 0, 0, 0);
}

}