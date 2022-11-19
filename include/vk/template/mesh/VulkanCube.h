#ifndef VULKAN_CUBE_H
#define VULKAN_CUBE_H

#include "MeshObject.h"
#include "vertex_struct.h"

namespace VulkanEngine {

class VULKANENGINE_EXPORT_API VulkanCube : public MeshObject {
 public:
  VulkanCube() = default;
  virtual ~VulkanCube() noexcept;

  void generateVertex() override;
  void updateVertex() override{};
  void setSize(float a, float b, float c) {
    m_a = a;
    m_b = b;
    m_c = c;
  }

 protected:
  float m_a = 1.f;
  float m_b = 1.f;
  float m_c = 1.f;
};

}  // namespace VulkanEngine

#endif /* VULKAN_CUBE_H */
