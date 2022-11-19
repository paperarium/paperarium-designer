#ifndef VULKAN_SHADER_H
#define VULKAN_SHADER_H

#include "VkObject.h"

namespace VulkanEngine {

class VULKANENGINE_EXPORT_API VulkanShader : public VkObject {
public:
  VulkanShader() = default;
  virtual ~VulkanShader();

  // interface
  virtual void prepare() override;
  virtual void update() override;
  virtual void prepareShaders() = 0;

  // getters
  VkPipeline &getPipeline() { return m_pipeline; }
  std::vector<VkPipelineShaderStageCreateInfo> &getShaderStages() { return m_shaderStages; }
  bool getDepthBiasEnabled() const { return m_depthBiasEnable; }
  bool isOneStage() const { return m_oneStage; }
  bool isInstanceShader() const { return m_instanceShader; }
  VkCullModeFlags getCullFlag() const { return m_cullFlag; }
  VkFrontFace getFrontFace() const { return m_frontFace; }
  VkPipelineVertexInputStateCreateInfo getVertexInputState() const { return m_inputState; }

  // setters
  void setCullFlag(VkCullModeFlags flag) { m_cullFlag = flag; }
  void setFrontFace(VkFrontFace face) { m_frontFace = face; }
  void setDepthBiasEnable(bool value) { m_depthBiasEnable = value; }
  void setOneStage(bool value) { m_oneStage = value; }
  void setVertexInputState(const VkPipelineVertexInputStateCreateInfo &inputStateCreateInfo) {
    m_inputState = inputStateCreateInfo;
    m_instanceShader = true;
  }

protected:
  VkPipelineShaderStageCreateInfo loadShader(const std::string &fileName, const VkShaderStageFlagBits &stage);

protected:
  VkPipeline m_pipeline = VK_NULL_HANDLE;
  std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;
  std::vector<VkShaderModule> m_shaderModules;

  VkCullModeFlags m_cullFlag = VK_CULL_MODE_NONE;
  VkFrontFace m_frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  VkPipelineVertexInputStateCreateInfo m_inputState;
  bool m_depthBiasEnable = false;
  bool m_instanceShader = false;
  bool m_oneStage = false;
};

}

#endif /* VULKAN_SHADER_H */
