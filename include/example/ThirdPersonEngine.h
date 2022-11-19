#ifndef THIRD_PERSON_ENGINE_H
#define THIRD_PERSON_ENGINE_H

#include "VulkanBaseEngine.h"
#include "camera/ThirdPersonCamera.h"

namespace VulkanEngine {

class VULKANENGINE_EXPORT_API ThirdPersonEngine : public VulkanBaseEngine {
 public:
  ThirdPersonEngine() = default;
  virtual ~ThirdPersonEngine() {}

  void updateCamera();

  ThirdPersonCamera m_camera;

 protected:
  float m_viewportSensitivity = 5.0f;
  float m_scrollSensitivity = 15.0f;
};

}  // namespace VulkanEngine

#endif /* THIRD_PERSON_ENGINE_H */
