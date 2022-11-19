#ifndef ASSIMP_MODEL_H
#define ASSIMP_MODEL_H

#include "ThirdPersonEngine.h"
#include "VulkanFrameBuffer.h"
#include "VulkanVertFragShader.h"
#include "camera/ShadowCamera.h"
#include "camera/UniformCamera.h"
#include "mesh/AssimpObject.h"
#include "mesh/VulkanCube.h"
#include "mesh/VulkanPlane.h"
#include "texture/VulkanTexture2D.h"

namespace VulkanEngine {

class AssimpModel : public ThirdPersonEngine {
 public:
  AssimpModel() = default;
  ~AssimpModel();

  void prepareFunctions() override;
  void prepareMyObjects() override;
  void buildMyObjects(VkCommandBuffer& cmd) override;
  void render() override;
  void setDescriptorSet();
  void createPipelines();
  void createCube();
  void createShadowFrameBuffer();
  void createDebugQuad();
  void buildCommandBuffersBeforeMainRenderPass(VkCommandBuffer& cmd) override;
  void seeDebugQuad();
  void OnUpdateUIOverlay(vks::UIOverlay* overlay){};

 protected:
  std::shared_ptr<AssimpObject> m_assimpObject = nullptr;

  // cube information for default cube
  std::shared_ptr<VulkanCube> m_cube = nullptr;
  std::shared_ptr<VulkanVertFragShader> m_cubeShader = nullptr;
  std::shared_ptr<UniformCamera> m_cubeUniform = nullptr;
  std::shared_ptr<VulkanTexture2D> m_cubeTextureA = nullptr;
  std::shared_ptr<VulkanTexture2D> m_cubeTextureB = nullptr;

  // debug plane + shader
  std::shared_ptr<VulkanPlane> m_debugPlane = nullptr;
  std::shared_ptr<VulkanVertFragShader> m_debugShader = nullptr;

  // shadow rendering
  std::shared_ptr<VulkanFrameBuffer> m_frameBuffer = nullptr;
  std::shared_ptr<VulkanVertFragShader> m_shadowShader = nullptr;
  std::shared_ptr<ShadowCamera> m_shadowCamera = nullptr;
  bool m_seeDebug = false;
};

}  // namespace VulkanEngine

#endif /* ASSIMP_MODEL_H */
