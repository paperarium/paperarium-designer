/*
 * VulkanPipelines.h
 * Created by: Evan Kirkiles
 * Created on: Fri Nov 4 2022
 * for Paperarium Design
 * 
 * Most, if not all code written by wjl:
 * https://github.com/engineer1109/LearnVulkan/blob/master/source/core/VulkanPipelines.h
 */
#ifndef VULKAN_PIPELINES_H
#define VULKAN_PIPELINES_H

#include "render_common.h"

#include "VulkanShader.h"

namespace VulkanEngine {

class VULKANENGINE_EXPORT_API VulkanPipelines {
public:
  VulkanPipelines(VkDevice &device);
  ~VulkanPipelines() = default;

  void createBasePipelineInfo(const VkPipelineLayout &pipelineLayout, const VkRenderPass &renderPass);
  void createPipeline(VulkanShader* shader, VkPolygonMode mode = VK_POLYGON_MODE_FILL);
  void createPipeline(VulkanShader* shader, VkRenderPass renderPass, VkPolygonMode mode = VK_POLYGON_MODE_FILL);
  void createPipeline(std::shared_ptr<VulkanShader> shader, VkPolygonMode mode = VK_POLYGON_MODE_FILL) {
    this->createPipeline(shader.get(), mode);
  }
  void createPipeline(std::shared_ptr<VulkanShader> shader, VkRenderPass renderPass, VkPolygonMode mode = VK_POLYGON_MODE_FILL) {
    this->createPipeline(shader.get(), renderPass, mode);
  }

public:
  VkDevice m_device;
  VkGraphicsPipelineCreateInfo m_pipelineCreateInfo;
  VkPipelineVertexInputStateCreateInfo m_vertexInputState;
  VkPipelineCache m_pipelineCache;

  VkPipelineInputAssemblyStateCreateInfo m_inputAssemblyState;
  VkPipelineRasterizationStateCreateInfo m_rasterizationState;
  VkPipelineColorBlendAttachmentState m_blendAttachmentState;
  VkPipelineColorBlendStateCreateInfo m_colorBlendState;
  VkPipelineDepthStencilStateCreateInfo m_depthStencilState;
  VkPipelineViewportStateCreateInfo m_viewportState;
  VkPipelineMultisampleStateCreateInfo m_multisampleState;
  std::vector<VkDynamicState> m_dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_LINE_WIDTH };
  VkPipelineDynamicStateCreateInfo m_dynamicState;

  std::array<VkPipelineShaderStageCreateInfo, 2> m_shaderStages;
};

}

#endif /*  VULKAN_CONTEXT_H  */