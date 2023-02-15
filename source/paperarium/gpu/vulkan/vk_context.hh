/*
 * vk_context.hh
 * Created by: Evan Kirkiles
 * Created on: Fri Dec 16 2022
 * for Paperarium
 */
#ifndef VK_CONTEXT_HH
#define VK_CONTEXT_HH

#include "gpu_common.h"
#include "gpu_context_private.hh"
#include "vk_device.hh"
#include "vk_swapchain.hh"
#include "vk_tools.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace paperarium::gpu {

class VKContext : public Context {
  friend class VKBackend;

 public:
  /** OS-specific window passed to constructor */
  VKContext(PLATF_SURF_PARAMS, bool debug);
  ~VKContext();

  virtual void activate() override{};
  virtual void deactivate() override{};
  virtual void begin_frame() override{};
  virtual void end_frame() override{};

  /* Will push all pending commands to the GPU. */
  virtual void flush() override{};
  /* Will wait until the GPU has finished executing all command. */
  virtual void finish() override{};

  static VKContext* get() { return static_cast<VKContext*>(Context::get()); }

  /* -------------------------------- PRIVATES --------------------------------
   */

 private:
  /** OS-specific window references */
  /* For example,
  HINSTANCE m_platform_handle; \
  HWND m_platform_window;
  */
  PLATF_SURF_MEMBERS

  // pretend to be Vulkan 1.0
  int const m_context_major_version = 1;
  int const m_context_minor_version = 0;
  bool m_debug = false;

  /** Vulkan API Resource Handles */
  VkInstance m_instance;
  VkPhysicalDevice m_physical_device;
  VkDevice m_device;
  VkQueue m_queue;
  VkCommandPool m_command_pool;
  vks::VKDevice* m_vulkan_device;  // wrapper around VkDevice

  /** Vulkan resource synchronization */
  std::vector<VkFence> m_in_flight_fences;
  std::vector<VkFence> m_in_flight_images;
  std::vector<VkSemaphore> m_image_available_semaphores;
  std::vector<VkSemaphore> m_render_finished_semaphores;

  /** Depth stencil (always only one) */
  struct {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory mem = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
  } m_depth_stencil;
  // TODO: Dynamically set depth format for depth + stencil from GPU?
  VkFormat m_depth_format = VK_FORMAT_D16_UNORM_S8_UINT;

  /** Vulkan image presentation */
  VkSurfaceKHR m_surface;
  vks::VKSwapchain m_swapchain;  // handles images and image views for us
  std::vector<VkFramebuffer> m_swapchain_framebuffers;
  std::vector<VkCommandBuffer> m_command_buffers;
  VkRenderPass m_render_pass;
  VkExtent2D m_render_extent;
  VkPipelineCache m_pipeline_cache;

  /** Required device extensions */
  std::vector<char const*> m_enabled_device_extensions;

  /** frame modulo swapchain_len. Used as index for sync objects. */
  int m_currentFrame = 0;
  /** Image index in the swapchain. Used as index for render objects. */
  uint32_t m_currentImage = 0;
  /** Used to unique framebuffer ids to return when swapchain is recreated. */
  uint32_t m_swapchain_id = 0;

  char const* getPlatformSpecificSurfaceExtension() const;

  // Vulkan instance initialization
  PAPER_TSuccess createInstance(bool use_window_surface = true);
  PAPER_TSuccess initSwapchain();
  PAPER_TSuccess pickPhysicalDevice();
  PAPER_TSuccess createLogicalDevice();

  // Vulkan component creation
  PAPER_TSuccess createSwapchain();
  PAPER_TSuccess createSynchronizationPrimitives();
  PAPER_TSuccess createFramebuffers();
  PAPER_TSuccess createCommandBuffers();
  PAPER_TSuccess createDepthStencil();
  PAPER_TSuccess createRenderPass();
  PAPER_TSuccess createPipelineCache();

  // Vulkan component manipulation
  PAPER_TSuccess recordCommandBuffers();
  PAPER_TSuccess recreateSwapchain();

  // Vulkan component destruction
  PAPER_TSuccess destroyCommandBuffers();
  PAPER_TSuccess destroyDepthStencil();
  PAPER_TSuccess destroyFramebuffers();
  PAPER_TSuccess destroySynchronizationPrimitives();
  PAPER_TSuccess destroySwapchain();

  // Single frame of render loop
  PAPER_TSuccess swapBuffers();
};

}  // namespace paperarium::gpu

#endif /* VK_CONTEXT_HH */
