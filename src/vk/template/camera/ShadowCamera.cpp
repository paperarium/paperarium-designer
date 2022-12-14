#include "camera/ShadowCamera.h"

namespace VulkanEngine {

/**
 * @brief Create the uniform buffer for the shadow camera
 */
void ShadowCamera::prepareUniformBuffers() {
  VK_CHECK_RESULT(m_context->vulkanDevice->createBuffer(
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      &m_uniformBuffer, sizeof(m_uboVS), &m_uboVS));
  VK_CHECK_RESULT(m_uniformBuffer.map());
  updateUniformBuffers();
}

void ShadowCamera::updateUniformBuffers() {
  // matrix from light's point of view
  glm::mat4 depthProjectionMatrix =
      glm::perspective(glm::radians(m_lightFOV), 1.0f, m_zNear, m_zFar);
  glm::mat4 depthViewMatrix =
      glm::lookAt(m_lightPos, glm::vec3(0.f), glm::vec3(0, 1, 0));
  glm::mat4 depthModelMatrix = glm::mat4(1.0f);
  m_uboVS.depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
  memcpy(m_uniformBuffer.mapped, &m_uboVS, sizeof(m_uboVS));
}

}  // namespace VulkanEngine