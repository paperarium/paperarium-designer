#ifndef VULKAN_BUFFER_H
#define VULKAN_BUFFER_H

#include "VkObject.h"
#include "VulkanBuffer.h"
#include "VulkanContext.h"

namespace VulkanEngine {

class VULKANENGINE_EXPORT_API VulkanBuffer : public VkObject {
 public:
  VulkanBuffer() = default;
  virtual ~VulkanBuffer();

  virtual void prepare() override;
  virtual void update() override;
  virtual void prepareUniformBuffers() = 0;
  virtual void updateUniformBuffers() = 0;

 public:
  vks::Buffer m_uniformBuffer;
};

}  // namespace VulkanEngine

#endif /* VULKAN_BUFFER_H */
