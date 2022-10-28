#ifndef VULKAN_OBJECT_H
#define VULKAN_OBJECT_H

#include "vulkan_macro.h"

namespace VulkanEngine {

interface VULKANENGINE_EXPORT_API VulkanObject {
public:
  VulkanObject() = default;
  virtual ~VulkanObject() = default;

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

#endif // VULKAN_OBJECT_H
