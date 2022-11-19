/*
 * VulkanBase.h
 * Created by: Evan Kirkiles
 * Created on: Fri Oct 28 2022
 * for Paperarium Design
 *
 * Code mostly written by wjl:
 * https://github.com/engineer1109/LearnVulkan/blob/master/source/base/VulkanBase.h
 */

#ifndef VULKANBASE_H
#define VULKANBASE_H

#include "VulkanDevice.hpp"
#include "VulkanSwapChain.h"
#include "VulkanTools.h"
#include "base_template.h"
#include "keycodes.hpp"
#include "render_common.h"
#include "vulkan_macro.h"

namespace VulkanEngine {

class VulkanBase {
 public:  // INIT METHODS
  VulkanBase() = default;
  virtual ~VulkanBase();

  void setWidth(uint32_t const& width);
  void setHeight(uint32_t const& height);

  void initVulkan();

  virtual void prepare();
  virtual void prepareBase();
  virtual void renderLoop();
  virtual void renderFrame();
  virtual void updateOverlay() {}
  virtual void render();
  virtual void draw();
  virtual void updateCommand(){};

  void waitForCurrentFrameComplete();
  void pause() { m_pause = true; }
  void resume() { m_pause = false; }
  void quit() { m_quit = true; }
  bool getPrepared() const { return m_prepared; }

 protected:  // INIT METHODS
  void createInstance();
  void pickPhysicalDevice();
  virtual void getDeviceFeatures(){};
  void createLogicalDevice();
  void initSwapchain();
  void createCommandPool();
  void createSwapChain();
  void createCommandBuffers();
  void createSynchronizationPrimitives();
  void createDepthStencil();
  void createRenderPass();
  void createPipelineCache();
  void createFramebuffers();
  virtual void buildCommandBuffers(){};
  void prepareFrame();
  void submitFrame();

 public:  // OPERATION METHODS
  void destroySurface();
  void destroyCommandBuffers();
  void windowResize();

  virtual void keyPressed(uint32_t) {}
  virtual void prepareFunctions() {}
  virtual void runFunction(int i);

  // Mouse listeners / handlers
  void handleMouseMove(float x, float y);
  void setMouseButtonLeft(bool value) { m_mouseButtons.left = value; }
  void setMouseButtonRight(bool value) { m_mouseButtons.right = value; }
  void setMouseButtonMiddle(bool value) { m_mouseButtons.middle = value; }

  // Sets the window id
  void setWindow(uint64_t winId) { m_winId = winId; }

  // destination width and height
  uint32_t m_destWidth = 1280;
  uint32_t m_destHeight = 720;

 protected:
  // Window / surface id
  uint64_t m_winId;

  // Viewport dimensions
  uint32_t m_width = 1280;
  uint32_t m_height = 720;

  // Vulkan instance states
  bool m_debug = true;
  bool m_stop = false;
  bool m_quit = false;
  bool m_pause = false;
  bool m_prepared = false;
  bool m_signalFrame = true;

  // Vulkan components
  VkResult m_result = VK_SUCCESS;
  VkInstance m_instance = VK_NULL_HANDLE;
  VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
  VkDevice m_device = VK_NULL_HANDLE;
  VkQueue m_queue = VK_NULL_HANDLE;
  VkCommandPool m_cmdPool = VK_NULL_HANDLE;

  VkFormat m_depthFormat = VK_FORMAT_D16_UNORM_S8_UINT;
  VkSubmitInfo m_submitInfo;
  VkPipelineStageFlags m_submitPipelineStages =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  // Physical device metadata
  VkPhysicalDeviceProperties m_deviceProperties;
  VkPhysicalDeviceFeatures m_deviceFeatures;
  VkPhysicalDeviceMemoryProperties m_deviceMemoryProperties;
  vks::VulkanDevice* m_vulkanDevice = nullptr;

  // Features / extensions enabled for our Vulkan instance
  std::vector<std::string> m_supportedInstanceExtensions;
  std::vector<std::string> m_supportedDeviceExtensions;
  std::vector<char const*> m_enabledDeviceExtensions;
  std::vector<char const*> m_enabledInstanceExtensions;
  VkPhysicalDeviceFeatures m_enabledFeatures;
  void* m_deviceCreatepNextChain = nullptr;

  // The swap chain for drawing to the screen
  VulkanSwapChain m_swapChain;

  struct RenderSemaphores {
    // Swap chain image presentation
    VkSemaphore presentComplete = VK_NULL_HANDLE;
    // Command buffer submission and execution
    VkSemaphore renderComplete = VK_NULL_HANDLE;
  } m_semaphores;

  struct {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory mem = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
  } m_depthStencil;

  // Fences for synchronizing CPU-GPU communication
  std::vector<VkFence> m_waitFences;

  // Render context
  std::vector<VkCommandBuffer> m_drawCmdBuffers;
  VkRenderPass m_renderPass;
  std::vector<VkFramebuffer> m_frameBuffers;
  uint32_t m_currentBuffer = 0;
  VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
  std::vector<VkShaderModule> m_shaderModules;
  VkPipelineCache m_pipelineCache;

  // Mouse positions
  glm::vec2 m_mousePos;
  glm::vec2 m_mousePosOld;
  float m_distance = 0.f;
  float m_oldDistance = 0.f;
  // Mouse button tracker
  struct MouseButton {
    bool left = false;
    bool right = false;
    bool middle = false;
  } m_mouseButtons;
  // Mouse scroll tracker
  struct Scroll {
    bool up = false;
    bool down = false;
  } m_scroll;

  // Qt settings (likely not to be used)
  struct Settings {
    bool fullscreen = false;
  } m_settings;

  std::string m_title = "Vulkan";
  std::vector<std::function<void()>> m_functions;

  // Measures time between frames in seconds
  float m_frameTimer = 0.f;
};

}  // namespace VulkanEngine

#endif  // VULKANBASE_H
