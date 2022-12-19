/*
 * vk_context.hh
 * Created by: Evan Kirkiles
 * Created on: Fri Dec 16 2022
 * for Paperarium
 */
#ifndef VK_CONTEXT_HH
#define VK_CONTEXT_HH

#include "vk_device.hh"
#include "vk_tools.h"
#include "vulkan/vulkan.h"

namespace paperarium::gpu {

class VKContext : public Context {
  friend class VKBackend;

 public:
  /** OS-specific window passed to constructor */
  VKContext(
#if defined(VK_USE_PLATFORM_WIN32_KHR)
      HINSTANCE platformHandle, HWND platformWindow,
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
      IDirectFB* dfb, IDirectFBSurface* window,
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
      wl_display* display, wl_surface* window,
#elif defined(VK_USE_PLATFORM_XCB_KHR)
      xcb_connection_t* connection, xcb_window_t window,
#elif defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK)
      void* view,
#elif defined(_DIRECT2DISPLAY) || defined(VK_USE_PLATFORM_HEADLESS_EXT)
      uint32_t width, uint32_t height,
#endif
      bool debug);
  ~VKContext();

  void activate() override;
  void deactivate() override;
  void begin_frame() override;
  void end_frame() override;

  static VKContext* get() { return static_cast<VKContext*>(Context::get()); }

  /* -------------------------------- PRIVATES --------------------------------
   */

 private:
  /** OS-specific window references */
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  HINSTANCE m_platform_handle;
  HWND m_platform_window;
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
  IDirectFB* m_platform_dfb;
  IDirectFBSurface *m_platform_window,
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
  wl_display* m_platform_display;
  wl_display* m_platform_window;
#elif defined(VK_USE_PLATFORM_XCB_KHR)
  xcb_connection_t* m_platform_connection;
  xcb_window_t m_platform_window;
#elif defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK)
  void* m_platform_view;
#elif defined(_DIRECT2DISPLAY) || defined(VK_USE_PLATFORM_HEADLESS_EXT)
  uint32_t m_platform_width;
  uint32_t m_platform_height;
#endif

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
  vks::VKDevice m_vulkan_device;  // wrapper around VkDevice

  /** Vulkan resource synchronization */
  std::vector<VkFence> m_in_flight_fences;
  std::vector<VkFence> m_in_flight_images;
  std::vector<VKSemaphore> m_image_available_semaphores;
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

  /** Required device extensions */
  std::vector<char const*> m_enabled_device_extensions;

  /** frame modulo swapchain_len. Used as index for sync objects. */
  int m_currentFrame = 0;
  /** Image index in the swapchain. Used as index for render objects. */
  uint32_t m_currentImage = 0;
  /** Used to unique framebuffer ids to return when swapchain is recreated. */
  uint32_t m_swapchain_id = 0;

  char const* getPlatformSpecificSurfaceExtension() const;
  PAPER_TSuccess createInstance(bool use_window_surface = true);
  PAPER_TSuccess initSwapchain();
  PAPER_TSuccess pickPhysicalDevice();
  PAPER_TSuccess createLogicalDevice();
  PAPER_TSuccess createSwapchain();
  PAPER_TSuccess createSynchronizationPrimitives();
  PAPER_TSuccess createFramebuffers();
  PAPER_TSuccess createCommandBuffers();
  PAPER_TSuccess recordCommandBuffers();
  PAPER_TSuccess createRenderPass();
  PAPER_TSuccess destroySwapchain();
};

}  // namespace paperarium::gpu

#endif /* VK_CONTEXT_HH */
