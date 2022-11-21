#include "mesh/AssimpObject.h"
#include "VulkanModel.hpp"

namespace VulkanEngine {

AssimpObject::~AssimpObject() {
  if (m_model) {
    m_model->destroy();
    delete_ptr(m_model);
  }
}

void AssimpObject::generateVertex() {
  m_model = new vks::Model();
  vks::VertexLayout layout =
      vks::VertexLayout({vks::Component::VERTEX_COMPONENT_POSITION,
                         vks::Component::VERTEX_COMPONENT_UVVEC4,
                         vks::Component::VERTEX_COMPONENT_NORMAL});
  m_model->loadFromFile(m_modelPath, layout, 1.f, m_context->vulkanDevice,
                        m_context->queue, nullptr);
  m_modelCenter = (m_model->dim.max + m_model->dim.min) * 0.5f;
}

void AssimpObject::build(VkCommandBuffer& cmdBuffer,
                         VulkanShader* vulkanShader) {
  VkDeviceSize offsets[1] = {0};
  if (vulkanShader->getPipeline()) {
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      vulkanShader->getPipeline());
  } else {
    std::cout << "Pipeline null, bind failed!" << std::endl;
  }
  vkCmdBindVertexBuffers(cmdBuffer, VERTEX_BUFFER_BIND_ID, 1,
                         &(m_model->vertices.buffer), offsets);
  vkCmdBindIndexBuffer(cmdBuffer, m_model->indices.buffer, 0,
                       VK_INDEX_TYPE_UINT32);
  vkCmdDrawIndexed(cmdBuffer, m_model->indexCount, 1, 0, 0, 0);
}

}  // namespace VulkanEngine