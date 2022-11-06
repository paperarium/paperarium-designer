#ifndef THIRD_PERSON_CAMERA_H
#define THIRD_PERSON_CAMERA_H

namespace VulkanEngine {

class VULKANENGINE_EXPORT_API ThirdPersonCamera {
public:

  ThirdPersonCamera() = default;
  ~ThirdPersonCamera() = default;

  void setZoom(float zoom) { m_zoom = zoom; }

  glm::vec3 m_rotation = glm::vec3();
  glm::vec3 m_cameraPos = glm::vec3();
  const float m_baseZoom = -2.f;
  float m_zoom = m_baseZoom;
  
};

}

#endif /* THIRD_PERSON_CAMERA_H */
