/*
 * Class wrapping access to the swap chain
 *
 * A swap chain is a collection of framebuffers used for rendering and
 * presentation to the windowing system
 *
 * Copyright (C) 2016-2017 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT)
 * (http://opensource.org/licenses/MIT)
 */
#ifndef VK_SWAPCHAIN_HH
#define VK_SWAPCHAIN_HH

#include "gpu_common.h"
#include "vk_tools.h"
#include <vulkan/vulkan.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#ifdef __APPLE__
#include <sys/utsname.h>
#endif

namespace vks {

typedef struct _SwapChainBuffers {
  VkImage image;
  VkImageView view;
} SwapChainBuffer;

class VKSwapchain {
 private:
  VkInstance instance;
  VkDevice device;
  VkPhysicalDevice physicalDevice;
  VkSurfaceKHR surface;
  // Function pointers
  PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
  PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR
      fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
  PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
  PFN_vkGetPhysicalDeviceSurfacePresentModesKHR
      fpGetPhysicalDeviceSurfacePresentModesKHR;
  PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
  PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
  PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
  PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
  PFN_vkQueuePresentKHR fpQueuePresentKHR;

 public:
  VkFormat colorFormat;
  VkColorSpaceKHR colorSpace;
  VkSwapchainKHR swapChain = VK_NULL_HANDLE;
  uint32_t imageCount;
  std::vector<VkImage> images;
  std::vector<SwapChainBuffer> buffers;
  uint32_t queueNodeIndex = UINT32_MAX;

  void initSurface(PLATF_SURF_PARAMS);
#if defined(_DIRECT2DISPLAY)
  void createDirect2DisplaySurface(uint32_t width, uint32_t height);
#endif
  void connect(VkInstance instance, VkPhysicalDevice physicalDevice,
               VkDevice device);
  void create(uint32_t* width, uint32_t* height, bool vsync = false,
              bool fullscreen = false);
  VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore,
                            uint32_t* imageIndex);
  VkResult queuePresent(VkQueue queue, uint32_t imageIndex,
                        VkSemaphore waitSemaphore = VK_NULL_HANDLE);
  void cleanup();
};

}  // namespace vks

#endif /* VK_SWAPCHAIN_HH */