#include "VulkanBaseEngine.h"

namespace VulkanEngine {

/* -------------------------------------------------------------------------- */
/*                        VULKAN ENGINE INITIALIZATION                        */
/* -------------------------------------------------------------------------- */
// Process:
//  1. prepareBase (from VulkanBase)
//  2. prepareDescriptorSets
//  3. prepareVertexDescriptions
//  4. prepareBasePipelines
//  5. prepareContext
//  6. prepareImGUI
//  7. prepareMyObjects
//  8. buildCommandBuffers

/**
 * @brief Sets up the base engine for rendering
 * 
 * Builds the base Vulkan instance, the descriptor sets and vertex descriptions,
 * the base pipelines, context, and the objects. Also builds command buffers.
 */
void VulkanBaseEngine::prepare() {
  prepareBase();
  prepareDescriptorSets();
  prepareVertexDescriptions();
  prepareBasePipelines();
  prepareContext();
  prepareImGui();
  prepareMyObjects(); // <-- this is overridden on a per-engine basis
  buildCommandBuffers();
  m_prepared = true;
}

/* ----------------------------- IMPLEMENTATION ----------------------------- */

/**
 * @brief Builds the Vulkan descriptor sets 
 * 
 * Think of a single descriptor as a handle or pointer into a resource. That
 * resource being a Buffer or a Image, and also holds other information, such as
 * the size of the buffer, or the type of sampler if it’s for an image. A
 * VkDescriptorSet is a pack of those pointers that are bound together.
 * 
 * A descriptor set essentially defines the interface through which we feed
 * data to our pipelines––that data being vertex positions / uvs / normals
 * or image / sampler information.
 */
void VulkanBaseEngine::prepareDescriptorSets() {
  m_vulkanDescriptorSet = new VulkanDescriptorSet(m_device, m_maxSets);
}

/**
 * @brief Builds the Vulkan vertex descriptions
 * 
 * A vertex description tells Vulkan how to interpret the data within the
 * descriptor sets, i.e. where to look for vertex positions, uvs, and normals.
 * This allows us to allocate the correct amount of memory for our descriptor
 * pool by combining our descriptor sets with our vertex descriptions.
 */
void VulkanBaseEngine::prepareVertexDescriptions() {
  m_vulkanVertexDescriptions = new VulkanVertexDescriptions();
  m_vulkanVertexDescriptions->GenerateTexVec4Descriptions();
}

/**
 * @brief Builds the Vulkan base pipelines
 * 
 * Pipelines are what take vertex/index buffer data and bring it all the way
 * to the frame buffer. These are where we load our shader modules, which have
 * slots that should be bound to the descriptor sets we defined before.
 * 
 * By putting vertex data into the descriptor sets, we enable the pipeline to
 * read from it and rasterize it into what you see on screen.
 */
void VulkanBaseEngine::prepareBasePipelines() {
  m_pipelines = new VulkanPipelines(m_device);
  m_pipelines->m_vertexInputState = m_vulkanVertexDescriptions->m_inputState;
  m_pipelines->m_pipelineCache = m_pipelineCache;
}

/**
 * @brief Creates the Vulkan context for all Vulkan objects
 * 
 * Populates the context with fields like the Vulkan device, command pool,
 * pipeline layout, pipeline cache, render pass, queue, and screen dimensions.
 */
void VulkanBaseEngine::prepareContext() {
  m_context = new VulkanContext();
  m_context->vulkanDevice = m_vulkanDevice;
  m_context->cmdPool = m_cmdPool;
  m_context->pPipelineLayout = &m_pipelineLayout;
  m_context->pipelineCache = m_pipelineCache;
  m_context->renderPass = m_renderPass;
  m_context->queue = m_queue;
  m_context->pScreenWidth = &m_width;
  m_context->pScreenHeight = &m_height;
}

/**
 * @brief Builds the UI Overlay pipeline for rendering GUI elements
 * 
 * Loads in the "uioverlay" shader module and builds it into a pipeline to then
 * render our ImGui elements with.
 */
void VulkanBaseEngine::prepareImGui() {
  if (m_settings.overlay) {
    m_UIOverlay.device = m_vulkanDevice;
    m_UIOverlay.queue = m_queue;
    m_UIOverlay.shaders = {
      loadShader(":/shaders/base/uioverlay.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
      loadShader(":/shaders/base/uioverlay.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT),
    };
    m_UIOverlay.prepareResources();
    m_UIOverlay.preparePipeline(m_pipelineCache, m_renderPass);
  }
}

/**
 * @brief Builds a command buffer containing our render pass
 * 
 * This command buffer defines the instructions to correctly draw our scene from
 * the descriptor sets we bind. 
 */
void VulkanBaseEngine::buildCommandBuffers() {
  VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
  for (size_t i = 0; i < m_drawCmdBuffers.size(); i++) {
    VK_CHECK_RESULT(vkBeginCommandBuffer(m_drawCmdBuffers[i], &cmdBufInfo));
    buildCommandBuffersBeforeMainRenderPass(m_drawCmdBuffers[i]);
    {
      VkClearValue clearValues[2];
      clearValues[0].color = {{0.1f, 0.2f, 0.3f, 1.0f}};
      clearValues[1].depthStencil = {1.0f, 0};
      // set target frame buffer
      VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
      renderPassBeginInfo.renderPass = m_renderPass;
      renderPassBeginInfo.renderArea.offset.x = 0;
      renderPassBeginInfo.renderArea.offset.y = 0;
      renderPassBeginInfo.renderArea.extent.width = m_width;
      renderPassBeginInfo.renderArea.extent.height = m_height;
      renderPassBeginInfo.clearValueCount = 2;
      renderPassBeginInfo.pClearValues = clearValues;
      renderPassBeginInfo.framebuffer = m_frameBuffers[i];
      // begin the render pass
      vkCmdBeginRenderPass(m_drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
      VkDeviceSize offsets[1] = {0};
      // bing our vertice descriptor sets to the pipeline
      vkCmdBindDescriptorSets(m_drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipelineLayout, 0, 1, &(m_vulkanDescriptorSet->get(0)), 0, NULL);

      /* ---------------------------- RENDER PASS --------------------------- */

      // 1. Set viewport and scissor
      setViewPorts(m_drawCmdBuffers[i]);
      // 2. Draw the objects in the scene
      buildMyObjects(m_drawCmdBuffers[i]);
      // 3. Draw the ImGUI interface on the surface
      drawUI(m_drawCmdBuffers[i]);

      /* ------------------------- END RENDER PASS -------------------------- */

      // end the render pass
      vkCmdEndRenderPass(m_drawCmdBuffers[i]);
    }
    buildCommandBuffersAfterMainRenderPass(m_drawCmdBuffers[i]);
    VK_CHECK_RESULT(vkEndCommandBuffer(m_drawCmdBuffers[i]));
  }
  vkQueueWaitIdle(m_queue);
}

/* ----------------------------- DRAW FUNCTIONS ----------------------------- */

/**
 * @brief Adds commands to set the viewport and scissor to a command buffer
 * 
 * These are used to set the dimensions of Vulkan's output at the beginning of
 * a command buffer.
 * 
 * @param commandBuffer - The command buffer being recorded 
 */
void VulkanBaseEngine::setViewPorts(VkCommandBuffer &commandBuffer) {
  VkViewport viewports[1];
  VkRect2D scissorRects[1];
  viewports[0] = { 0, 0, float(m_width), float(m_height), 0.0, 1.0};
  scissorRects[0] = vks::initializers::rect2D(m_width, m_height, 0, 0);
  vkCmdSetViewport(commandBuffer, 0, 1, viewports);
  vkCmdSetScissor(commandBuffer, 0, 1, scissorRects);
}

/**
 * @brief Adds commands to draw the Dear ImGui UI to a command buffer
 * 
 * @param commandBuffer - The command buffer being recorded
 */
void VulkanBaseEngine::drawUI(const VkCommandBuffer commandBuffer) {
  if (m_settings.overlay) {
    const VkViewport viewport = vks::initializers::viewport((float)m_width, (float)m_height, 0.0f, 1.0f);
    const VkRect2D scissor = vks::initializers::rect2D(m_width, m_height, 0, 0);
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    m_UIOverlay.draw(commandBuffer);
  }
}

/* ------------------------ META-LEVEL IMPLEMENTATION ----------------------- */


/* -------------------------------------------------------------------------- */
/*                             VULKAN DESTRUCTION                             */
/* -------------------------------------------------------------------------- */
// Vulkan engine components are all managed with RAII, so we are safe to delete
// them. Only thing we need to worry about is pipeline layout.

/**
 * @brief Destroy the Vulkan Base Engine:: Vulkan Base Engine object
 * 
 * Frees the descriptor set, vertex, descriptions, pipelines, and context pointers.
 * Then deletes the pipeline layout from Vulkan.
 */
VulkanBaseEngine::~VulkanBaseEngine() {
  delete_ptr(m_vulkanDescriptorSet);
  delete_ptr(m_vulkanVertexDescriptions);
  delete_ptr(m_pipelines);
  delete_ptr(m_context);
  VK_SAFE_DELETE(m_pipelineLayout, vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr));
}

/* -------------------------------------------------------------------------- */
/*                              VULKAN RENDERING                              */
/* -------------------------------------------------------------------------- */
// Rendering abstracts the low-level calls that Vulkan Base does to construct
// a simple render loop that continually runs to render frames to the surface.

/**
 * @brief The base engine render function (OVERRIDE THIS). 
 */
void VulkanBaseEngine::render() {};

/**
 * @brief Begins an asynchronous render thread
 * 
 * This will likely be replaced by an implementation that works with signals and
 * slots on a QThread. We don't want to have to manage all of the platform-specific
 * mouse and other events ourselves, so we can just read them from Qt and have
 * a more platform-agnostic codebase.
 */
void VulkanBaseEngine::renderAsyncThread() {
  m_thread = new std::thread(&VulkanBaseEngine::renderLoop, this);
}

/**
 * @brief Joins the render thread back into the main thread, ending the loop.
 * 
 * Deletes the now-joined thread as well.
 */
void VulkanBaseEngine::renderJoin() {
  m_thread->join();
  delete_ptr(m_thread);
}

/* -------------------------------------------------------------------------- */
/*                              ImGui MANAGEMENT                              */
/* -------------------------------------------------------------------------- */
// Manages the ImGui instance, which draws a little window within the Vulkan
// surface to provide information.

/**
 * @brief Parse events we've received into the ImGui instance
 */
void VulkanBaseEngine::updateOverlay() {
  if (!m_settings.overlay) return;

  // synchronize ImGui IO
  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = ImVec2((float)m_width, (float)m_height);
  io.DeltaTime = m_frameTimer;
  io.MousePos = ImVec2(m_mousePos.x, m_mousePos.y);
  io.MouseDown[0] = m_mouseButtons.left;
  io.MouseDown[1] = m_mouseButtons.right;

  // build our ImGui overlay here
  // ImGui::NewFrame();
  // DO STUFF FOR IMGUI
}

/* -------------------------------------------------------------------------- */
/*                                MISCELLANEOUS                               */
/* -------------------------------------------------------------------------- */
// Not sure where to put these functions, or they're potentially going to
// be phased out––partticularly initWindow.

/**
 * @brief Calls "setupWindow"
 * 
 * We don't actually create our own window, so this might not be necessary? We
 * just want to link our Vulkan instance to the surface in Qt.
 * On devices using XCB (Unix), also initializes the XCB connection.
 */
void VulkanBaseEngine::initWindow() {
#if defined(VK_USE_PLATFORM_XCB_KHR)
  initxcbConnection();
  setupWindow();
#elif defined(_WIN32)
  setupWindow();
#endif
}

void VulkanBaseEngine::updateCommand() {
  if (m_rebuild) {
    buildCommandBuffers();
    m_rebuild = false;
  }
}

/**
 * @brief Loads a shader into the engine.
 * 
 * @param fileName 
 * @param stage 
 * @return VkPipelineShaderStageCreateInfo 
 */
VkPipelineShaderStageCreateInfo VulkanBaseEngine::loadShader(
  const std::string &resourcePath, const VkShaderStageFlagBits &stage) {
  VkPipelineShaderStageCreateInfo shaderStage = {};
  shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStage.stage = stage;
  shaderStage.module = vks::tools::loadShader(resourcePath.c_str(), m_context->getDevice());
  shaderStage.pName = "main";
  assert(shaderStage.module != VK_NULL_HANDLE);
  m_shaderModules.push_back(shaderStage.module);
  return shaderStage;
}

}
