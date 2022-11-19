#ifndef VULKAN_VERT_FRAG_SHADER_H
#define VULKAN_VERT_FRAG_SHADER_H

#include "VulkanShader.h"

namespace VulkanEngine {

class VULKANENGINE_EXPORT_API VulkanVertFragShader : public VulkanShader {
public:
  VulkanVertFragShader() = default;
  ~VulkanVertFragShader() = default;

  void setShaderObjPath(const std::string &vertPath, const std::string &fragPath) {
    m_vertPath = vertPath;
    m_fragPath = fragPath;
  }

  virtual void prepareShaders() override {
    m_shaderStages.push_back(loadShader(m_vertPath.c_str(), VK_SHADER_STAGE_VERTEX_BIT));
    m_shaderStages.push_back(loadShader(m_fragPath.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT));
  }

protected:
  std::string m_vertPath;
  std::string m_fragPath;
};

}

#endif /* VULKAN_VERT_FRAG_SHADER_H */
