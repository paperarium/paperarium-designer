/*
 * VulkanDescriptorSet.h
 * Created by: Evan Kirkiles
 * Created on: Fri Nov 4 2022
 * for Paperarium Design
 *
 * Most, if not all code written by wjl:
 * https://github.com/engineer1109/LearnVulkan/blob/master/source/core/VulkanDescriptorSet.h
 */
#ifndef VULKAN_DESCRIPTOR_SET_H
#define VULKAN_DESCRIPTOR_SET_H

#include "VulkanInitializers.hpp"
#include "VulkanTools.h"
#include "render_common.h"
#include "vulkan_macro.h"

namespace VulkanEngine {

/**
 * @brief A wrapper around the VkDescriptorSet type
 *
 * Read more: https://vkguide.dev/docs/chapter-4/descriptors/
 * Allows for bundling CPU resources together in a way that is accessible to
 * the GPU. Supports two types of descriptors: buffers (vertices, etc.) and
 * images (samplers) which are then usable on the GPU.
 */
class VULKANENGINE_EXPORT_API VulkanDescriptorSet {
 public:
  enum class DescriptorType { IMAGE = 0, BUFFER = 1 };

  struct DescriptorInfo {
    uint32_t binding = 0;
    VkDescriptorImageInfo* descriptorImageInfo = nullptr;
    VkDescriptorBufferInfo* descriptorBufferInfo = nullptr;
    VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    int descriptorIndex = 0;
    DescriptorType type = DescriptorType::IMAGE;
  };

 public:
  VulkanDescriptorSet(VkDevice device, int maxSets = 1);
  ~VulkanDescriptorSet();

  void addBinding(uint32_t binding, VkDescriptorImageInfo* descriptorInfo,
                  VkDescriptorType descriptorType,
                  VkShaderStageFlags stageFlags, int descriptorIndex);
  void addBinding(uint32_t binding, VkDescriptorBufferInfo* descriptorInfo,
                  VkDescriptorType descriptorType,
                  VkShaderStageFlags stageFlags, int descriptorIndex);

  void GenPipelineLayout(VkPipelineLayout* pipelineLayout);
  VkDescriptorSet& get(int index);
  size_t getSize();

 protected:
  VkDevice m_device = VK_NULL_HANDLE;
  std::vector<VkDescriptorSet> m_descriptorSets;
  std::vector<DescriptorInfo> m_descriptorInfos;
  VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
  VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
};

}  // namespace VulkanEngine

#endif /*  VULKAN_DESCRIPTOR_SET_H  */