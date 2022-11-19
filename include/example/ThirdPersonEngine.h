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
};

}  // namespace VulkanEngine

#endif /* THIRD_PERSON_ENGINE_H */
