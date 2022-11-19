#ifndef UNIFORM_CAMERA_H
#define UNIFORM_CAMERA_H

#include "VulkanBuffer.h"

namespace VulkanEngine {

class VULKANENGINE_EXPORT_API UniformCamera : public VulkanBuffer {
public:

  struct CameraMatrix {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 normal;
    glm::vec4 lightpos;
  };

public:

  UniformCamera() = default;
  ~UniformCamera() = default;

  virtual void prepareUniformBuffers() override;
  virtual void updateUniformBuffers() override;

public:

  CameraMatrix m_uboVS;
  glm::vec3 *m_pRotation = nullptr;
  glm::vec3 *m_pCameraPos = nullptr;
  float *m_pZoom = nullptr;

};

}

#endif /* UNIFORM_CAMERA_H */
