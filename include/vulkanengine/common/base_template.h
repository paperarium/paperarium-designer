#ifndef VULKAN_BASE_TEMPLATE_H
#define VULKAN_BASE_TEMPLATE_H

#include "render_common.h"
#include "vulkan_macro.h"

namespace VulkanEngine {

/**
 * @brief Safely deletes a pointer if it is not null
 * 
 * @tparam T 
 * @param ptr 
 */
template<class T>
void delete_ptr(T* &ptr) {
  if (ptr) {
    delete ptr;
    ptr = nullptr;
  }
}

/**
 * @brief Safely deletes an array pointer if it is not null
 * 
 * @tparam T 
 * @param ptr 
 */
template<class T>
void delete_array(T* &ptr) {
  if (ptr) {
    delete[] ptr;
    ptr = nullptr;
  }
}

/**
 * @brief Clamps a value between a min and a max
 * 
 * @tparam T 
 * @param ptr 
 */
template<class T>
void clamp(T value, T minValue, T maxValue) {
  T res = std::min(maxValue, value);
  res = std::max(minValue, value);
  return res;
}

}

#endif /*  VULKAN_BASE_TEMPLATE_H  */