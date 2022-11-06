#ifndef VULKAN_BUFFER_H
#define VULKAN_BUFFER_H

#include "VkObject.h"
#include "VulkanContext.h"
#include "VulkanBuffer.h"

namespace VulkanEngine {

class VULKANENGINE_EXPORT_API VulkanBuffer: public VkObject {
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

}

#endif /* VULKAN_BUFFER_H */
