#ifndef ASSIMP_OBJECT_H
#define ASSIMP_OBJECT_H

#include "MeshObject.h"
#include "VulkanModel.hpp"

namespace VulkanEngine {

class AssimpObject : public MeshObject {
 public:
  AssimpObject() = default;
  virtual ~AssimpObject();

  void setModelPath(std::string const& modelPath) { m_modelPath = modelPath; }
  void generateVertex() override;
  void updateVertex() override{};

  virtual void build(VkCommandBuffer& cmdBuffer,
                     VulkanShader* vulkanShader) override;
  virtual void build(VkCommandBuffer& cmdBuffer,
                     std::shared_ptr<VulkanShader> vulkanShader) override {
    this->build(cmdBuffer, vulkanShader.get());
  }

 protected:
  std::string m_modelPath;
  vks::Model* m_model = nullptr;
};

}  // namespace VulkanEngine

#endif /* ASSIMP_OBJECT_H */
