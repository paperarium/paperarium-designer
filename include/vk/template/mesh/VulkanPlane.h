#ifndef VULKAN_PLANE_H
#define VULKAN_PLANE_H

#include "MeshObject.h"
#include "vertex_struct.h"

namespace VulkanEngine {

class VULKANENGINE_EXPORT_API VulkanPlane : public MeshObject {
 public:
  VulkanPlane() = default;
  ~VulkanPlane() noexcept;

  void generateVertex() override;
  void updateVertex() override{};
  void setSize(float a, float b) {
    m_a = a;
    m_b = b;
  }

 protected:
  float m_a = 1.f;
  float m_b = 1.f;
};

}  // namespace VulkanEngine

#endif /* VULKAN_PLANE_H */
