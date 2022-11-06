#ifndef TRIANGLE_UNIFORM_H
#define TRIANGLE_UNIFORM_H

#include "camera/UniformCamera.h"

namespace VulkanEngine {

class TriangleUniform: public UniformCamera {
public:

  TriangleUniform() = default;
  ~TriangleUniform() = default;

  virtual void updateUniformBuffers() override;

public:
  glm::vec3 *m_pRotation = nullptr;
  glm::vec3 *m_pCameraPos = nullptr;
  float *m_pZoom = nullptr;
};

}

#endif /* TRIANGLE_UNIFORM_H */
