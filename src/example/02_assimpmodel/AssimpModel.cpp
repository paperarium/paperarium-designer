#include "02_assimpmodel/AssimpModel.h"

namespace VulkanEngine {

AssimpModel::~AssimpModel() noexcept { destroyObjects(); }

void AssimpModel::prepareFunctions() {
  m_functions.emplace_back([this] { seeDebugQuad(); });
}

void AssimpModel::prepareMyObjects() {
  m_camera.m_zoom = -4.f;
  createCube();
  createShadowFrameBuffer();
  createDebugQuad();
  setDescriptorSet();
  createPipelines();
}

void AssimpModel::buildMyObjects(VkCommandBuffer& cmd) {
  m_assimpObject->build(cmd, m_cubeShader);
  m_assimpObject->build(cmd, m_lineShader);
  if (m_seeDebug) {
    m_debugPlane->build(cmd, m_debugShader);
  }
}

void AssimpModel::render() {
  updateCamera();
  m_cubeUniform->update();
  m_shadowCamera->update();
}

void AssimpModel::setDescriptorSet() {
  m_vulkanDescriptorSet->addBinding(
      0, &(m_cubeUniform->m_uniformBuffer.descriptor),
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
  m_vulkanDescriptorSet->addBinding(1, &(m_cubeTextureA->descriptor),
                                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                    VK_SHADER_STAGE_FRAGMENT_BIT, 0);
  m_vulkanDescriptorSet->addBinding(2, &(m_cubeTextureB->descriptor),
                                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                    VK_SHADER_STAGE_FRAGMENT_BIT, 0);
  m_vulkanDescriptorSet->addBinding(3, &(m_frameBuffer->getDescriptor()),
                                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                    VK_SHADER_STAGE_FRAGMENT_BIT, 0);
  m_vulkanDescriptorSet->addBinding(
      4, &(m_shadowCamera->m_uniformBuffer.descriptor),
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
  m_vulkanDescriptorSet->GenPipelineLayout(&m_pipelineLayout);
}

void AssimpModel::createPipelines() {
  m_pipelines->createBasePipelineInfo(m_pipelineLayout, m_renderPass);
  m_pipelines->createPipeline(m_cubeShader);
  m_pipelines->createPipeline(m_lineShader, VK_POLYGON_MODE_LINE);
  m_pipelines->createPipeline(m_debugShader);
  m_pipelines->createPipeline(m_shadowShader,
                              m_frameBuffer->getRenderPass()->get());
}

void AssimpModel::createCube() {
  REGISTER_OBJECT<AssimpObject>(m_assimpObject);
  m_assimpObject->setModelPath(
      "/Users/evan/Desktop/evan/paperarium/paperarium-designer/test/models/"
      "lloid.obj");
  m_assimpObject->prepare();

  REGISTER_OBJECT<VulkanVertFragShader>(m_cubeShader);
  m_cubeShader->setShaderObjPath(":/shaders/02_assimpmodel/scene.vert.spv",
                                 ":/shaders/02_assimpmodel/scene.frag.spv");
  m_cubeShader->setCullFlag(VK_CULL_MODE_BACK_BIT);
  m_cubeShader->setFrontFace(VK_FRONT_FACE_CLOCKWISE);
  m_cubeShader->prepare();

  REGISTER_OBJECT<VulkanVertFragShader>(m_lineShader);
  m_lineShader->setShaderObjPath(":/shaders/02_assimpmodel/line.vert.spv",
                                 ":/shaders/02_assimpmodel/line.frag.spv");
  m_lineShader->setCullFlag(VK_CULL_MODE_BACK_BIT);
  m_lineShader->setFrontFace(VK_FRONT_FACE_CLOCKWISE);
  m_lineShader->prepare();

  REGISTER_OBJECT<UniformCamera>(m_cubeUniform);
  m_cubeUniform->m_uboVS.lightpos = glm::vec4(10.0f, -10.0f, 10.0f, 1.0f);
  m_cubeUniform->m_pCameraPos = m_assimpObject->getCenter();
  m_cubeUniform->m_pRotation = &m_camera.m_rotation;
  m_cubeUniform->m_pZoom = &m_camera.m_zoom;
  m_cubeUniform->prepare();

  REGISTER_OBJECT<VulkanTexture2D>(m_cubeTextureA);
  m_cubeTextureA->loadFromFile(
      "/Users/evan/Desktop/evan/paperarium/paperarium-designer/test/textures/"
      "sobj_hnw_rent.png",
      VK_FORMAT_R8G8B8A8_UNORM);

  REGISTER_OBJECT<VulkanTexture2D>(m_cubeTextureB);
  m_cubeTextureB->loadFromFile(
      "/Users/evan/Desktop/evan/paperarium/paperarium-designer/test/textures/"
      "container.png",
      VK_FORMAT_R8G8B8A8_UNORM);
}

void AssimpModel::createShadowFrameBuffer() {
  m_frameBuffer = std::make_shared<VulkanFrameBuffer>();
  m_frameBuffer->setVulkanDevice(m_vulkanDevice);
  m_frameBuffer->setFormat(VK_FORMAT_D16_UNORM);
  m_frameBuffer->setSize(4096, 4096);
  m_frameBuffer->createWithDepth();

  REGISTER_OBJECT<VulkanVertFragShader>(m_shadowShader);
  m_shadowShader->setShaderObjPath(":/shaders/02_assimpmodel/shadow.vert.spv",
                                   ":/shaders/02_assimpmodel/shadow.frag.spv");
  m_shadowShader->setCullFlag(VK_CULL_MODE_FRONT_BIT);
  m_shadowShader->setDepthBiasEnable(true);
  m_shadowShader->setOneStage(true);
  m_shadowShader->prepare();

  REGISTER_OBJECT<ShadowCamera>(m_shadowCamera);
  m_shadowCamera->m_lightPos = m_cubeUniform->m_uboVS.lightpos;
  m_shadowCamera->prepare();
}

void AssimpModel::createDebugQuad() {
  REGISTER_OBJECT(m_debugPlane);
  m_debugPlane->setSize(0.5, 0.5);
  m_debugPlane->prepare();

  REGISTER_OBJECT<VulkanVertFragShader>(m_debugShader);
  m_debugShader->setShaderObjPath(":/shaders/02_assimpmodel/quad.vert.spv",
                                  ":/shaders/02_assimpmodel/quad.frag.spv");
  m_debugShader->setCullFlag(VK_CULL_MODE_NONE);
  m_debugShader->prepare();
}

void AssimpModel::buildCommandBuffersBeforeMainRenderPass(
    VkCommandBuffer& cmd) {
  VkClearValue clearValues[2];
  clearValues[0].depthStencil = {1.0f, 0};

  // begin recording the command buffers for the render pass
  VkRenderPassBeginInfo renderPassBeginInfo =
      vks::initializers::renderPassBeginInfo();
  renderPassBeginInfo.renderPass = m_frameBuffer->getRenderPass()->get();
  renderPassBeginInfo.framebuffer = m_frameBuffer->get();
  renderPassBeginInfo.renderArea.extent.width = m_frameBuffer->getWidth();
  renderPassBeginInfo.renderArea.extent.height = m_frameBuffer->getHeight();
  renderPassBeginInfo.clearValueCount = 1;
  renderPassBeginInfo.pClearValues = clearValues;
  vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

  // set the viewport (dynamic)
  auto viewport = vks::initializers::viewport((float)m_frameBuffer->getWidth(),
                                              (float)m_frameBuffer->getHeight(),
                                              0.0f, 1.0f);
  vkCmdSetViewport(cmd, 0, 1, &viewport);

  // set the scissor (dynamic)
  auto scissor = vks::initializers::rect2D(m_frameBuffer->getWidth(),
                                           m_frameBuffer->getHeight(), 0, 0);
  vkCmdSetScissor(cmd, 0, 1, &scissor);

  // set depth bias (aka polygon effect)
  // required to avoid shadow mapping artifacts

  // constant depth bias factor
  float depthBiasConstant = 1.25f;
  // slope depth bias factor, applied depending on polygon's slope
  float depthBiasSlope = 1.75f;
  vkCmdSetDepthBias(cmd, depthBiasConstant, 0.0f, depthBiasSlope);

  // bind the descriptor sets
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_pipelineLayout, 0, 1,
                          &(m_vulkanDescriptorSet->get(0)), 0, NULL);
  // attach the ASSIMP object to the scene
  m_assimpObject->build(cmd, m_shadowShader);
  vkCmdEndRenderPass(cmd);
}

void AssimpModel::seeDebugQuad() {
  m_seeDebug = !m_seeDebug;
  m_rebuild = true;
}

// void AssimpModel::OnUpdateUIOverlay(vks::UIOverlay* overlay) {
//   // if (m_UIOverlay.button(""))
// }

}  // namespace VulkanEngine