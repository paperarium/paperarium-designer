#ifndef SHADOW_CAMERA_H
#define SHADOW_CAMERA_H

#include "VulkanBuffer.h"

namespace VulkanEngine {

class VULKANENGINE_EXPORT_API ShadowCamera : public VulkanBuffer {
 public:
  struct ShadowMVP {
    glm::mat4 depthMVP;
  };

 public:
  ShadowCamera() = default;
  ~ShadowCamera() = default;

  virtual void prepareUniformBuffers() override;
  virtual void updateUniformBuffers() override;

 public:
  ShadowMVP m_uboVS;
  float m_lightFOV = 45.f;
  float m_zNear = 1.f;
  float m_zFar = 96.f;
  glm::vec3 m_lightPos = glm::vec3(0.f);
};

}  // namespace VulkanEngine

#endif /* SHADOW_CAMERA_H */
