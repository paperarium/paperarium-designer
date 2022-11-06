#ifndef VK_OBJECT_H
#define VK_OBJECT_H

#include "vulkan_macro.h"
#include "base_template.h"
#include "filesystem_utils.h"
#include "VulkanDevice.hpp"
#include "VulkanContext.h"

namespace VulkanEngine {

class VULKANENGINE_EXPORT_API VkObject {
public:
  VkObject() = default;
  virtual ~VkObject() = default;

  template<class T>
  static std::shared_ptr<T> New(VulkanContext* context) {
    std::shared_ptr<T> object = std::make_shared<T>();
    object->setContext(context);
    return object;
  }

  virtual void setContext(VulkanContext* context) {
    m_context = context;
  }

  virtual void prepare() = 0;
  virtual void update() = 0;

protected:
  VulkanContext* m_context = nullptr;
};
  
}

#endif /* VK_OBJECT_H */