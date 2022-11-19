/*
 * VulkanVertexDescriptions.h
 * Created by: Evan Kirkiles
 * Created on: Fri Nov 4 2022
 * for Paperarium Design
 *
 * Most, if not all code written by wjl:
 * https://github.com/engineer1109/LearnVulkan/blob/master/source/core/VulkanVertexDescriptions.h
 */
#ifndef VULKAN_VERTEX_DESCRIPTIONS_H
#define VULKAN_VERTEX_DESCRIPTIONS_H

#include "VulkanInitializers.hpp"
#include "base_template.h"
#include "render_common.h"
#include "vertex_struct.h"

namespace VulkanEngine {

class VULKANENGINE_EXPORT_API VulkanVertexDescriptions {
 public:
  VulkanVertexDescriptions() = default;
  ~VulkanVertexDescriptions() = default;

  VkPipelineVertexInputStateCreateInfo m_inputState;
  std::vector<VkVertexInputBindingDescription> m_inputBinding;
  std::vector<VkVertexInputAttributeDescription> m_inputAttributes;

  void GenerateUVDescriptions() {
    // Binding description
    m_inputBinding.resize(1);
    m_inputBinding[0] = vks::initializers::vertexInputBindingDescription(
        VERTEX_BUFFER_BIND_ID, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);

    // Attribute descriptions
    // describes memory layout and shader positions
    m_inputAttributes.resize(3);
    // location 0: position
    m_inputAttributes[0] = vks::initializers::vertexInputAttributeDescription(
        VERTEX_BUFFER_BIND_ID, 0, VK_FORMAT_R32G32B32_SFLOAT,
        offsetof(Vertex, pos));
    // location 1: texture coordinates
    m_inputAttributes[1] = vks::initializers::vertexInputAttributeDescription(
        VERTEX_BUFFER_BIND_ID, 1, VK_FORMAT_R32G32_SFLOAT,
        offsetof(Vertex, uv));
    // location 2: vertex normal
    m_inputAttributes[2] = vks::initializers::vertexInputAttributeDescription(
        VERTEX_BUFFER_BIND_ID, 2, VK_FORMAT_R32G32B32_SFLOAT,
        offsetof(Vertex, normal));

    // create the vertex input state from our attribute descriptions
    m_inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    m_inputState.vertexBindingDescriptionCount =
        static_cast<uint32_t>(m_inputBinding.size());
    m_inputState.pVertexBindingDescriptions = m_inputBinding.data();
    m_inputState.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(m_inputAttributes.size());
    m_inputState.pVertexAttributeDescriptions = m_inputAttributes.data();
  }

  void GenerateUVWDescriptions() {
    // Binding description
    m_inputBinding.resize(1);
    m_inputBinding[0] = vks::initializers::vertexInputBindingDescription(
        VERTEX_BUFFER_BIND_ID, sizeof(VertexUVW), VK_VERTEX_INPUT_RATE_VERTEX);

    // Attribute descriptions
    // describes memory layout and shader positions
    m_inputAttributes.resize(3);
    // location 0: position
    m_inputAttributes[0] = vks::initializers::vertexInputAttributeDescription(
        VERTEX_BUFFER_BIND_ID, 0, VK_FORMAT_R32G32B32_SFLOAT,
        offsetof(VertexUVW, pos));
    // location 1: texture coordinates (UVW)
    m_inputAttributes[1] = vks::initializers::vertexInputAttributeDescription(
        VERTEX_BUFFER_BIND_ID, 1, VK_FORMAT_R32G32B32_SFLOAT,
        offsetof(VertexUVW, uv));
    // location 2: vertex normal
    m_inputAttributes[2] = vks::initializers::vertexInputAttributeDescription(
        VERTEX_BUFFER_BIND_ID, 2, VK_FORMAT_R32G32B32_SFLOAT,
        offsetof(VertexUVW, normal));

    // create the vertex input state from our attribute descriptions
    m_inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    m_inputState.vertexBindingDescriptionCount =
        static_cast<uint32_t>(m_inputBinding.size());
    m_inputState.pVertexBindingDescriptions = m_inputBinding.data();
    m_inputState.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(m_inputAttributes.size());
    m_inputState.pVertexAttributeDescriptions = m_inputAttributes.data();
  }

  void GenerateTexVec4Descriptions() {
    // Binding description
    m_inputBinding.resize(1);
    m_inputBinding[0] = vks::initializers::vertexInputBindingDescription(
        VERTEX_BUFFER_BIND_ID, sizeof(VertexTexVec4),
        VK_VERTEX_INPUT_RATE_VERTEX);

    // Attribute descriptions
    // describes memory layout and shader positions
    m_inputAttributes.resize(3);
    // location 0: position
    m_inputAttributes[0] = vks::initializers::vertexInputAttributeDescription(
        VERTEX_BUFFER_BIND_ID, 0, VK_FORMAT_R32G32B32_SFLOAT,
        offsetof(VertexTexVec4, pos));
    // location 1: texture coordinates (TexVec4)
    m_inputAttributes[1] = vks::initializers::vertexInputAttributeDescription(
        VERTEX_BUFFER_BIND_ID, 1, VK_FORMAT_R32G32B32A32_SFLOAT,
        offsetof(VertexTexVec4, uv));
    // location 2: vertex normal
    m_inputAttributes[2] = vks::initializers::vertexInputAttributeDescription(
        VERTEX_BUFFER_BIND_ID, 2, VK_FORMAT_R32G32B32_SFLOAT,
        offsetof(VertexTexVec4, normal));

    // create the vertex input state from our attribute descriptions
    m_inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    m_inputState.vertexBindingDescriptionCount =
        static_cast<uint32_t>(m_inputBinding.size());
    m_inputState.pVertexBindingDescriptions = m_inputBinding.data();
    m_inputState.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(m_inputAttributes.size());
    m_inputState.pVertexAttributeDescriptions = m_inputAttributes.data();
  }
};

}  // namespace VulkanEngine

#endif /*  VULKAN_VERTEX_DESCRIPTIONS_H  */