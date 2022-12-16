#include "VulkanShader.h"

namespace VulkanEngine {

/**
 * @brief Destroy the Vulkan Shader:: Vulkan Shader object
 *
 * Frees this shader's pipeline and all of its shader modules.
 */
VulkanShader::~VulkanShader() {
  VK_SAFE_DELETE(m_pipeline, vkDestroyPipeline(m_context->getDevice(),
                                               m_pipeline, nullptr));
  for (auto& shaderModule : m_shaderModules) {
    VK_SAFE_DELETE(shaderModule, vkDestroyShaderModule(m_context->getDevice(),
                                                       shaderModule, nullptr));
  }
}

void VulkanShader::prepare() { prepareShaders(); }
void VulkanShader::update() {}

/**
 * @brief Loads a shader from a file
 *
 * @param fileName The path to the shader file
 * @param stage Which stage this shader should be a component in
 * @return VkPipelineShaderStageCreateInfo
 */
VkPipelineShaderStageCreateInfo VulkanShader::loadShader(
    std::string const& fileName, VkShaderStageFlagBits const& stage) {
  VkPipelineShaderStageCreateInfo shaderStage = {};
  shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStage.stage = stage;
  shaderStage.module =
      vks::tools::loadShader(fileName.c_str(), m_context->getDevice());
  shaderStage.pName = "main";  // make this a param in the future
  // assert(shaderStage.module != VK_NULL_HANDLE);
  m_shaderModules.push_back(shaderStage.module);
  return shaderStage;
}

}  // namespace VulkanEngine