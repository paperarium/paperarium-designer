#include "VulkanRenderPass.h"
#include "VulkanTools.h"

namespace VulkanEngine {

VulkanRenderPass::~VulkanRenderPass() {
  if (m_renderPass) {
    vkDestroyRenderPass(m_device, m_renderPass, nullptr);
  }
}

/**
 * @brief Builds a render pass with color + depth attachments
 */
void VulkanRenderPass::createColorDepthPass() {
  VkAttachmentDescription osAttachments[2] = {};
  // color attachment
  osAttachments[0].format = m_format;
  osAttachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  osAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  osAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  osAttachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  osAttachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  osAttachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  osAttachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  // depth attachment
  osAttachments[1].format = m_depthFormat;
  osAttachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
  osAttachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  osAttachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  osAttachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  osAttachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  osAttachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  osAttachments[1].finalLayout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // build attachment references
  VkAttachmentReference colorReference = {};
  colorReference.attachment = 0;
  colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  VkAttachmentReference depthReference = {};
  depthReference.attachment = 1;
  depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // build subpass description
  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorReference;
  subpass.pDepthStencilAttachment = &depthReference;

  // build subpass dependencies
  // SUBPASS: external > color > depth > external
  // STAGE (execution): frag > color > depth > frag
  // ACCESS (memory): shader read > color > depth > shader read
  std::array<VkSubpassDependency, 2> dependencies;
  // color
  dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
  dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
  // depth
  dependencies[1].srcSubpass = 0;
  dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  // build the render pass
  VkRenderPassCreateInfo renderPassCreateInfo =
      vks::initializers::renderPassCreateInfo();
  renderPassCreateInfo.attachmentCount = 2;
  renderPassCreateInfo.pAttachments = osAttachments;
  renderPassCreateInfo.subpassCount = 1;
  renderPassCreateInfo.pSubpasses = &subpass;
  renderPassCreateInfo.dependencyCount =
      static_cast<uint32_t>(dependencies.size());
  renderPassCreateInfo.pDependencies = dependencies.data();
  VK_CHECK_RESULT(vkCreateRenderPass(m_device, &renderPassCreateInfo, nullptr,
                                     &m_renderPass));
}

void VulkanRenderPass::createDepthPass() {
  // build attachment description
  VkAttachmentDescription attachmentDescription = {};
  attachmentDescription.format = m_format;
  attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
  // clear depth at beginning of render pass
  attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  // we will read from depth, so store the results
  attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  // we don't care about initial layout of the attachment
  attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  // attachment will be transitioned to shader read at render pass end
  attachmentDescription.finalLayout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

  // build the references to the depth attachment for the subpass
  VkAttachmentReference depthReference = {};
  depthReference.attachment = 0;
  depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // build the subpass description
  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 0;
  subpass.pDepthStencilAttachment = &depthReference;

  // use subpass dependencies for layout transitions
  std::array<VkSubpassDependency, 2> dependencies;
  // early fragment test
  dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
  dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
  // depth stencil
  dependencies[1].srcSubpass = 0;
  dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  // build the render pass
  VkRenderPassCreateInfo renderPassCreateInfo =
      vks::initializers::renderPassCreateInfo();
  renderPassCreateInfo.attachmentCount = 1;
  renderPassCreateInfo.pAttachments = &attachmentDescription;
  renderPassCreateInfo.subpassCount = 1;
  renderPassCreateInfo.pSubpasses = &subpass;
  renderPassCreateInfo.dependencyCount =
      static_cast<uint32_t>(dependencies.size());
  renderPassCreateInfo.pDependencies = dependencies.data();
  VK_CHECK_RESULT(vkCreateRenderPass(m_device, &renderPassCreateInfo, nullptr,
                                     &m_renderPass));
}

}  // namespace VulkanEngine