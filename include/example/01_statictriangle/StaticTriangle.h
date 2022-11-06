#ifndef STATIC_TRIANGLE_H
#define STATIC_TRIANGLE_H

#include "VulkanBaseEngine.h"
#include "01_statictriangle/objects/Triangle.h"
#include "01_statictriangle/objects/TriangleShader.h"
#include "01_statictriangle/objects/TriangleUniform.h"
#include "camera/ThirdPersonCamera.h"

namespace VulkanEngine {

class StaticTriangle: public VulkanBaseEngine {
public:

  StaticTriangle() = default;
  virtual ~StaticTriangle();

public:

  void prepareMyObjects() override;
  void buildMyObjects(VkCommandBuffer &cmd) override;
  void render() override;
  void setDescriptorSet();
  void createPipelines();
  void createTriangle();

private:
  std::shared_ptr<Triangle> m_triangle = nullptr;
  std::shared_ptr<TriangleShader> m_triangleShader = nullptr;
  std::shared_ptr<TriangleUniform> m_triangleUniform = nullptr;

  ThirdPersonCamera m_camera;

};

}

#endif /* STATIC_TRIANGLE_H */
