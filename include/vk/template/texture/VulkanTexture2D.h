#ifndef VULKAN_TEXTURE2D_H
#define VULKAN_TEXTURE2D_H

#include "VulkanTexture.h"

namespace VulkanEngine {

class VULKANENGINE_EXPORT_API VulkanTexture2D : public VulkanTexture {
 public:
  VulkanTexture2D() = default;
  virtual ~VulkanTexture2D() = default;

  void loadFromFile(
      std::string file, VkFormat format, vks::VulkanDevice* device,
      VkQueue copyQueue,
      VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
      VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      bool forceLinear = false);

  void loadFromFile(
      std::string file, VkFormat format,
      VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
      VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      bool forceLinear = false) {
    if (m_context) {
      loadFromFile(file, format, m_context->vulkanDevice, m_context->queue,
                   imageUsageFlags, imageLayout, forceLinear);
    }
  }
};

}  // namespace VulkanEngine

#endif /* VULKAN_TEXTURE2D_H */
