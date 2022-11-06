#include "VulkanDescriptorSet.h"

namespace VulkanEngine {

/**
 * @brief Construct a new Vulkan Descriptor Set:: Vulkan Descriptor Set object
 * 
 * @param device The device which will read from the descriptor
 * @param maxSets The maximum number of sets in this descriptor
 */
VulkanDescriptorSet::VulkanDescriptorSet(VkDevice device, int maxSets) {
  m_device = device;
  m_descriptorSets.resize(maxSets);
}

/**
 * @brief Destroy the Vulkan Descriptor Set:: Vulkan Descriptor Set object
 * 
 * Deallocates the descriptor sets, descriptor pool, and descriptor set layout
 * referenced in this object.
 */
VulkanDescriptorSet::~VulkanDescriptorSet() {
  for (auto &descriptorSet : m_descriptorSets)
    VK_SAFE_DELETE(descriptorSet, vkFreeDescriptorSets(m_device, m_descriptorPool, 1, &descriptorSet));
  VK_SAFE_DELETE(m_descriptorPool, vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr));
  VK_SAFE_DELETE(m_descriptorSetLayout, vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr));
}

/**
 * @brief Adds an image (sampler) binding to the descriptor set
 * 
 * @param binding 
 * @param descriptorInfo 
 * @param descriptorType 
 * @param stageFlags 
 * @param descriptorIndex 
 */
void VulkanDescriptorSet::addBinding(uint32_t binding, VkDescriptorImageInfo *descriptorInfo,
    VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, int descriptorIndex) {
  DescriptorInfo descInfo = {};
  descInfo.binding = binding;
  descInfo.descriptorImageInfo = descriptorInfo;
  descInfo.descriptorBufferInfo = nullptr;
  descInfo.descriptorType = descriptorType;
  descInfo.stageFlags = stageFlags;
  descInfo.descriptorIndex = descriptorIndex;
  descInfo.type = DescriptorType::IMAGE;
  m_descriptorInfos.push_back(descInfo);
}

/**
 * @brief Adds a buffer (vertices, etc.) binding to the descriptor set
 * 
 * @param binding 
 * @param descriptorInfo 
 * @param descriptorType 
 * @param stageFlags 
 * @param descriptorIndex 
 */
void VulkanDescriptorSet::addBinding(uint32_t binding, VkDescriptorBufferInfo *descriptorInfo,
    VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, int descriptorIndex) {
  DescriptorInfo descInfo = {};
  descInfo.binding = binding;
  descInfo.descriptorImageInfo = nullptr;
  descInfo.descriptorBufferInfo = descriptorInfo;
  descInfo.descriptorType = descriptorType;
  descInfo.stageFlags = stageFlags;
  descInfo.descriptorIndex = descriptorIndex;
  descInfo.type = DescriptorType::BUFFER;
  m_descriptorInfos.push_back(descInfo);
}

/**
 * @brief Generates a pipeline layout for the descriptor sets
 * 
 * Read more: https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineLayout.html
 * "Zero or more descriptor set layouts and zero or more push constant ranges 
 * are combined to form a pipeline layout object describing the complete set of
 * resources that *can* be accessed by a pipeline. The pipeline layout represents
 * a sequence of descriptor sets with each having a specific layout. This sequence
 * of layouts is used to determine the interface between shader stages and shader
 * resources. Each pipeline is created using a pipeline layout."
 * 
 * @param pipelineLayout 
 */
void VulkanDescriptorSet::GenPipelineLayout(VkPipelineLayout *pipelineLayout) {
  // initialize all of our descriptor set layouts
  std::vector<VkDescriptorPoolSize> poolSizes;
  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
  std::vector<std::vector<VkWriteDescriptorSet>> writeDescriptorSets(m_descriptorSets.size());
  for (auto &descriptorInfo: m_descriptorInfos) {
    poolSizes.push_back(vks::initializers::descriptorPoolSize(descriptorInfo.descriptorType, 1));
    if (descriptorInfo.descriptorIndex == 0) {
      setLayoutBindings.push_back(
        vks::initializers::descriptorSetLayoutBinding(
          descriptorInfo.descriptorType, descriptorInfo.stageFlags, descriptorInfo.binding));
    }
    if (descriptorInfo.type == DescriptorType::IMAGE) {
      writeDescriptorSets[descriptorInfo.descriptorIndex].push_back(
        vks::initializers::writeDescriptorSet(
          nullptr, descriptorInfo.descriptorType, descriptorInfo.binding, descriptorInfo.descriptorImageInfo)
      );
    } else {
      writeDescriptorSets[descriptorInfo.descriptorIndex].push_back(
        vks::initializers::writeDescriptorSet(
          nullptr, descriptorInfo.descriptorType, descriptorInfo.binding, descriptorInfo.descriptorBufferInfo)
      );
    }
  }
  // initialize a descriptor pool for all of our descriptor sets
  VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(
    static_cast<uint32_t>(poolSizes.size()), poolSizes.data(), m_descriptorSets.size());
  VK_CHECK_RESULT(vkCreateDescriptorPool(m_device, &descriptorPoolInfo, nullptr, &m_descriptorPool));
  // determine a descriptor set layout based on the descriptor pool
  VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(
    setLayoutBindings.data(), static_cast<uint32_t>(setLayoutBindings.size()));
  VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_device, &descriptorLayout, nullptr, &m_descriptorSetLayout));
  // now use descriptor set layout to build our pipeline layout
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&m_descriptorSetLayout, 1);
  VkPushConstantRange pushConstantRange = vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), 0);
  pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
  pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
  VK_CHECK_RESULT(vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, nullptr, pipelineLayout));
  
  // now, using our pipeline layoutm, allocate our descriptor sets
  for (int i = 0; i < m_descriptorSets.size(); i++) {
    VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(m_descriptorPool, &m_descriptorSetLayout, 1);
    VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device, &allocInfo, &m_descriptorSets[i]));
    // save the locations of the descriptor sets for later use
    for (size_t j = 0; j < writeDescriptorSets[i].size(); j++)
      writeDescriptorSets[i][j].dstSet = m_descriptorSets[i];
    vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writeDescriptorSets[i].size()), writeDescriptorSets[i].data(), 0, NULL);
  }
}

/**
 * @brief Retrieves the descriptor set at index i
 * 
 * @param index 
 * @return VkDescriptorSet& 
 */
VkDescriptorSet &VulkanDescriptorSet::get(int index) {
  if (index < m_descriptorSets.size()) {
    return m_descriptorSets[index];
  } else {
    LOGI("VulkanDescriptorSet|get| DescriptorSets Out of Range");
    return m_descriptorSets[0];
  }
}

/**
 * @brief Returns the number of descriptor sets allocated.
 * 
 * @return size_t 
 */
size_t VulkanDescriptorSet::getSize() {
  return m_descriptorSets.size();
}

}
