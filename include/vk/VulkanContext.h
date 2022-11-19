/*
 * VulkanContext.h
 * Created by: Evan Kirkiles
 * Created on: Fri Oct 28 2022
 * for Paperarium Design
 *
 * Most, if not all code written by wjl:
 * https://github.com/engineer1109/LearnVulkan/blob/master/source/core/VulkanContext.h
 */
#ifndef VULKAN_CONTEXT_H
#define VULKAN_CONTEXT_H

#include "VulkanDevice.hpp"
#include "render_common.h"

namespace VulkanEngine {

/**
 * @brief The basic Vulkan context exposed as an API
 *
 */
struct VULKANENGINE_EXPORT_API VulkanContext {
  vks::VulkanDevice* vulkanDevice = nullptr;
  VkCommandBuffer copyCmd = VK_NULL_HANDLE;
  VkCommandPool cmdPool = VK_NULL_HANDLE;
  VkPipelineLayout* pPipelineLayout = nullptr;
  VkPipelineCache pipelineCache = VK_NULL_HANDLE;
  VkRenderPass renderPass = VK_NULL_HANDLE;
  VkQueue queue = VK_NULL_HANDLE;
  uint32_t* pScreenWidth = nullptr;
  uint32_t* pScreenHeight = nullptr;

  VkDevice& getDevice() { return vulkanDevice->logicalDevice; }

  VkPhysicalDevice& getPhysicalDevice() { return vulkanDevice->physicalDevice; }
};

}  // namespace VulkanEngine

#endif /* VULKAN_CONTEXT_H */
