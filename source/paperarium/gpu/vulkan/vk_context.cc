/*
 * vk_context.cc
 * Created by: Evan Kirkiles
 * Created on: Fri Dec 16 2022
 * for Paperarium
 */

#include "vk_context.hh"
#include "vk_tools.h"
#include <array>

// declare some helpers which exist at the end of the file
static std::vector<VkLayerProperties> layers_get_available();
static bool layers_enable_layer(
    std::vector<VkLayerProperties>& layers_available,
    std::vector<char const*>& layers_enabled, char const* layer_name);
static std::vector<VkExtensionProperties> exts_get_available();
static bool exts_enable_ext(
    std::vector<VkExtensionProperties>& extensions_available,
    std::vector<char const*>& extensions_enabled, char const* ext_name);
static bool exts_device_supports(VkPhysicalDevice device,
                                 std::vector<char const*>& extensions_enabled);

/* -------------------------------- VKContext ------------------------------- */
// ! TODO: Destructurs for all of the things initialized in drawing context

namespace paperarium::gpu {

/* Triple buffering. */
int const MAX_FRAMES_IN_FLIGHT = 2;

/**
 * @brief Initializes the VKContext::VKContext object
 *
 * Sets member variables related to the surface to be drawn to.
 */
VKContext::VKContext(PLATF_SURF_PARAMS, bool debug)
    : Context(),
      PLATF_SURF_INITIALIZERS,
      m_debug(debug),
      m_instance(VK_NULL_HANDLE),
      m_physical_device(VK_NULL_HANDLE),
      m_device(VK_NULL_HANDLE),
      m_queue(VK_NULL_HANDLE),
      m_command_pool(VK_NULL_HANDLE),
      m_surface(VK_NULL_HANDLE),
      m_render_pass(VK_NULL_HANDLE),
      m_swapchain() {
  bool const use_window_surface = true;  // keep this on for now (headless?)

  // build the vulkan instance on the surface
  PAPER_CHECK_RESULT(createInstance(use_window_surface))
  if (use_window_surface) initSwapchain();
  PAPER_CHECK_RESULT(pickPhysicalDevice());
  PAPER_CHECK_RESULT(createLogicalDevice());
  PAPER_CHECK_RESULT(createSwapchain());
  PAPER_CHECK_RESULT(createCommandBuffers());
  PAPER_CHECK_RESULT(createSynchronizationPrimitives());
  PAPER_CHECK_RESULT(createDepthStencil());
  PAPER_CHECK_RESULT(createRenderPass());
  PAPER_CHECK_RESULT(createPipelineCache());
  PAPER_CHECK_RESULT(createFramebuffers());
}

/**
 * @brief Destroy the VKContext::VKContext object
 *
 * Entirely deletes the Vulkan instance and all of its components.
 */
VKContext::~VKContext() {
  if (m_queue != VK_NULL_HANDLE) VK_CHECK_RESULT(vkQueueWaitIdle(m_queue));
  if (m_device != VK_NULL_HANDLE) VK_CHECK_RESULT(vkDeviceWaitIdle(m_device));
  destroySwapchain();
  VK_SAFE_DELETE(m_render_pass,
                 vkDestroyRenderPass(m_device, m_render_pass, NULL));
  VK_SAFE_DELETE(m_command_pool,
                 vkDestroyCommandPool(m_device, m_command_pool, NULL));
  VK_SAFE_DELETE(m_device, vkDestroyDevice(m_device, NULL));
  VK_SAFE_DELETE(m_surface, vkDestroySurfaceKHR(m_instance, m_surface, NULL));
  VK_SAFE_DELETE(m_instance, vkDestroyInstance(m_instance, NULL));
  delete m_vulkan_device;
}

/* -------------------------------------------------------------------------- */
/*                               DRAWING CONTEXT                              */
/* -------------------------------------------------------------------------- */
// Process (meta Vulkan):
//   1.  createInstance  <------
//   2.  pickPhysicalDevice    |
//   3.  createLogicalDevice   | (init surface before device to be able to
// Swapchain process:          |  ensure our picked GPU supports its format)
//   2.  initSwapchain ---------
//   3.  createCommandPool (done for us in VkDevice)
//   4.  createSwapChain
//   5.  createCommandBuffers
//   6.  createSynchronizationPrimitives
//   7.  createDepthStencil (! not implemented currently)
//   8.  createRenderPass
//   9.  createPipelineCache
//   10. createFramebuffers
// (See constructor for function calls)

/* -------------------------------------------------------------------------- */
/*                                  INSTANCE                                  */
/* -------------------------------------------------------------------------- */
// Handles all management of the instance and devices. Order:
//  instance -> physical device -> logical device

/**
 * @brief Builds the Vulkan instance
 *
 * Applies platform-specific extensions as well as a validation layer (if we
 * are using debug mode). Handles OS-specific extension creation.
 */
PAPER_TSuccess VKContext::createInstance(bool use_window_surface) {
  // create the vulkan app information
  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Paperarium Design";
  appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
  appInfo.pEngineName = "Paperarium Design";
  appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
  appInfo.apiVersion = VK_MAKE_API_VERSION(0, m_context_major_version,
                                           m_context_major_version, 0);

  // initialize the Vulkan instance info, which may be modified below
  VkInstanceCreateInfo instanceCreateInfo = {};
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pNext = NULL;
  instanceCreateInfo.pApplicationInfo = &appInfo;
  instanceCreateInfo.flags = 0;

  /* -------------------------- INSTANCE EXTENSIONS ------------------------- */

  // get the available layers and extensions
  m_enabled_device_extensions.resize(0);
  auto layers_av = layers_get_available();
  auto ext_av = exts_get_available();
  std::vector<char const*> layers_enbl;
  std::vector<char const*> ext_enbl;

  // if debugging, add khronos validation layer and debug extensions
  if (m_debug) {
    char const* validation_layer_name = "VK_LAYER_KHRONOS_validation";
    layers_enable_layer(layers_av, layers_enbl, validation_layer_name);
    exts_enable_ext(ext_av, ext_enbl, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    exts_enable_ext(ext_av, ext_enbl, VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
  }

  // add OS-specific surface instance extension
  if (use_window_surface) {
    char const* native_surface_ext = getPlatformSpecificSurfaceExtension();
    exts_enable_ext(ext_av, ext_enbl, VK_KHR_SURFACE_EXTENSION_NAME);
    exts_enable_ext(ext_av, ext_enbl, native_surface_ext);
    m_enabled_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  }

// When running on iOS/macOS with MoltenVK and VK_KHR_portability_enumeration
// is defined and supported by the instance, enable the extension and flag.
// Also enable K_KHR_get_physical_device_properties2 if not already enabled
// (required by VK_KHR_portability_subset)
#if defined(VK_USE_PLATFORM_MACOS_MVK)
  exts_enable_ext(ext_av, ext_enbl,
                  VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
  exts_enable_ext(ext_av, ext_enbl,
                  VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
  instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
  m_enabled_device_extensions.push_back("VK_KHR_portability_subset");
#endif

  // finally, add the extensions and layer information to the instance creator
  instanceCreateInfo.ppEnabledLayerNames = layers_enbl.data();
  instanceCreateInfo.enabledLayerCount =
      static_cast<uint32_t>(layers_enbl.size());
  instanceCreateInfo.ppEnabledExtensionNames = ext_enbl.data();
  instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ext_enbl.size());

  // and build the instance!
  VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, NULL, &m_instance));
  return PAPER_kSuccess;
}

/**
 * @brief Returns the Vulkan extension needed for surfaces on each platform
 * @return char const* A VK_KHR_<PLATFORM>_EXTENSION_NAME
 */
char const* VKContext::getPlatformSpecificSurfaceExtension() const {
#if defined(_WIN32)
  return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#elif defined(_DIRECT2DISPLAY)
  return VK_KHR_DISPLAY_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
  return VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_XCB_KHR)
  return VK_KHR_XCB_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
  return VK_MVK_MACOS_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_HEADLESS_EXT)
  return VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME;
#endif
}

/* -------------------------------------------------------------------------- */
/*                          PHYSICAL + LOGICAL DEVICE                         */
/* -------------------------------------------------------------------------- */

/**
 * @brief Picks a GPU to use for the Vulkan instance
 *
 * @return PAPER_TSuccess if everything worked.
 */
PAPER_TSuccess VKContext::pickPhysicalDevice() {
  m_physical_device = VK_NULL_HANDLE;
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(m_instance, &device_count, NULL);
  std::vector<VkPhysicalDevice> physical_devices(device_count);
  vkEnumeratePhysicalDevices(m_instance, &device_count,
                             physical_devices.data());

  /* ----------------------------- GPU SELECTION ---------------------------- */
  // sort GPUs based on score for our application. then choose best one.

  int best_device_score = -1;
  for (auto const& physical_device : physical_devices) {
    // get the device properties & features
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(physical_device, &device_properties);
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physical_device, &features);
    DEBUG_PRINTF("%s : \n", device_properties.deviceName);

    // check if the device supports our extensions
    if (!exts_device_supports(physical_device, m_enabled_device_extensions)) {
      DEBUG_PRINTF(" - Device does not support required device extensions.\n");
      continue;
    }
    // check if the device supports presentation
    if (m_surface != VK_NULL_HANDLE) {
      uint32_t format_count;
      vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, m_surface,
                                           &format_count, NULL);
      uint32_t present_count;
      vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, m_surface,
                                                &present_count, NULL);
      // for now, anything will do
      if (format_count == 0 || present_count == 0) {
        DEBUG_PRINTF(" - Device does not support presentation.\n");
        continue;
      }
    }
    // check if the device supports specific features
    /** Needed for wide lines emulation and barycentric coords, etc... */
    if (!features.geometryShader)
      DEBUG_PRINTF(" - Device does not support geometryShader.\n");

#if STRICT_REQUIREMENTS
    // if we can't work without those features, continue on
    if (!features.geometryShader || !features.dualSrcBlend ||
        !features.logicOp) {
      continue;
    }
#endif

    // now score the device
    int device_score = 0;
    switch (device_properties.deviceType) {
      case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        device_score = 400;
        break;
      case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        device_score = 300;
        break;
      case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        device_score = 200;
        break;
      case VK_PHYSICAL_DEVICE_TYPE_CPU:
        device_score = 100;
        break;
      default:
        device_score = 0;
        break;
    }
    if (device_score > best_device_score) {
      m_physical_device = physical_device;
      best_device_score = device_score;
    }
    DEBUG_PRINTF(" - Device suitable.\n");
  }

  // if no device was picked, we have no Vulkan device
  if (m_physical_device == VK_NULL_HANDLE) {
    LOG("Error: No suitable Vulkan Device found!\n");
    return PAPER_kFailure;
  }
  return PAPER_kSuccess;
}

/**
 * @brief Create logical representation of our physical device.
 *
 * Reads information from the physical device to create a logical
 * representation. This is then used to find a valid depth format and allows us
 * to create our presentation and command semaphores for synchronizing draw
 * calls and command consumption so that:
 *  - the image is displayed before we start submitting new commands to the
 * queue
 *  - the image is not presented until all commands have been submitted and
 * executed
 */
PAPER_TSuccess VKContext::createLogicalDevice() {
  // initialize a VulkanDevice from the physical device data
  m_vulkan_device = new vks::VKDevice(m_physical_device);

  // set our device features for the logical device
  VkPhysicalDeviceFeatures device_features = {};
#if STRICT_REQUIREMENTS
  device_features.geometryShader = VK_TRUE;
#endif
  VkResult result = m_vulkan_device->createLogicalDevice(
      device_features, m_enabled_device_extensions, NULL);
  m_device = (VkDevice)(*m_vulkan_device);
  // get the device graphics queue
  uint32_t graphics_queue_i =
      m_vulkan_device->getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
  vkGetDeviceQueue(m_device, graphics_queue_i, 0, &m_queue);

  // link the Vulkan instance's swapchain from logical to physical device
  m_swapchain.connect(m_instance, m_physical_device, m_device);
  return PAPER_kSuccess;
}

/* -------------------------------------------------------------------------- */
/*                                  SWAPCHAIN                                 */
/* -------------------------------------------------------------------------- */
// Handles all management of the swapchain and its frame buffers. Order:
//  swapchain -> command buffers -> framebuffers -> semaphores

/**
 * @brief Initializes the swapchain for the surface, if we need it
 * @return PAPER_TSuccess Always
 */
PAPER_TSuccess VKContext::initSwapchain() {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  m_swapchain.initSurface(m_platform_handle, m_platform_window);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
  m_swapchain.initSurface(m_platform_dfb, m_platform_window);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
  m_swapchain.initSurface(m_platform_display, m_platform_window);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
  m_swapchain.initSurface(m_platform_connection, m_platform_window);
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
  m_swapchain.initSurface(m_platform_view);
#elif (defined(_DIRECT2DISPLAY) || defined(VK_USE_PLATFORM_HEADLESS_EXT))
  m_swapchain.initSurface(m_platform_width, m_platform_height);
#endif
  return PAPER_kSuccess;
}

/**
 * @brief Creates the Vulkan swapchain
 *
 * This is re-called on every window resize event, as these invalidate our
 * swapchains and their components.
 *
 * @return PAPER_TSuccess
 */
PAPER_TSuccess VKContext::createSwapchain() {
  if (m_device != VK_NULL_HANDLE) VK_CHECK_RESULT(vkDeviceWaitIdle(m_device));

  // nab the capabilities of the physical device to check surface size
  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, m_surface,
                                            &capabilities);
  m_render_extent = capabilities.currentExtent;
  // if window size was weird, limit it
  if (m_render_extent.width == UINT32_MAX) {
    // Window Manager is going to set the surface size based on the given size.
    // Choose something between minImageExtent and maxImageExtent.
    m_render_extent.width = 1280;
    m_render_extent.height = 720;
    if (capabilities.minImageExtent.width > m_render_extent.width) {
      m_render_extent.width = capabilities.minImageExtent.width;
    }
    if (capabilities.minImageExtent.height > m_render_extent.height) {
      m_render_extent.height = capabilities.minImageExtent.height;
    }
  }

  // increment swapchain count for unique framebuffer id's
  m_swapchain_id++;
  m_swapchain.create(&m_render_extent.width, &m_render_extent.height);
  m_in_flight_images.resize(m_swapchain.imageCount);
  m_swapchain_framebuffers.resize(m_swapchain.imageCount);
  return PAPER_kSuccess;
}

/**
 * @brief Destroys the swapchain used in this instance
 *
 * Cleans up all semaphores, frame buffers, and command buffers. This is called
 * on every window resize.
 */
PAPER_TSuccess VKContext::destroySwapchain() {
  m_swapchain.cleanup();
  destroySynchronizationPrimitives();
  destroyCommandBuffers();
  destroyDepthStencil();
  destroyFramebuffers();
  return PAPER_kSuccess;
}

/* -------------------------------------------------------------------------- */
/*                               COMMAND BUFFERS                              */
/* -------------------------------------------------------------------------- */

/**
 * @brief Creates a command buffer for each swapchain image
 *
 * These are created in the graphics queue family.
 * @return PAPER_TSuccess always.
 */
PAPER_TSuccess VKContext::createCommandBuffers() {
  // resize command buffers to fit swapchain's image count
  m_command_buffers.resize(m_swapchain.imageCount);
  m_vulkan_device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                       m_command_buffers.data(),
                                       m_swapchain.imageCount);
  return PAPER_kSuccess;
}

/**
 * @brief Destroys the command buffers
 * @return PAPER_TSuccess Success always
 */
PAPER_TSuccess VKContext::destroyCommandBuffers() {
  for (auto command_buffer : m_command_buffers)
    vkFreeCommandBuffers(m_device, m_command_pool, 1, &command_buffer);
  return PAPER_kSuccess;
}

/* -------------------------------------------------------------------------- */
/*                         SYNCHRONIZATION PRIMITIVES                         */
/* -------------------------------------------------------------------------- */

PAPER_TSuccess VKContext::createSynchronizationPrimitives() {
  // create the synchronization primitives for each frame in flight
  m_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
  m_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
  m_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
  for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VK_CHECK_RESULT(vkCreateSemaphore(m_device, &semaphore_info, NULL,
                                      &m_image_available_semaphores[i]));
    VK_CHECK_RESULT(vkCreateSemaphore(m_device, &semaphore_info, NULL,
                                      &m_render_finished_semaphores[i]));
    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VK_CHECK_RESULT(
        vkCreateFence(m_device, &fence_info, NULL, &m_in_flight_fences[i]));
  }
  return PAPER_kSuccess;
}

PAPER_TSuccess VKContext::destroySynchronizationPrimitives() {
  m_in_flight_images.resize(0);
  for (auto semaphore : m_image_available_semaphores)
    vkDestroySemaphore(m_device, semaphore, NULL);
  for (auto semaphore : m_render_finished_semaphores)
    vkDestroySemaphore(m_device, semaphore, NULL);
  for (auto fence : m_in_flight_fences) vkDestroyFence(m_device, fence, NULL);
  for (auto framebuffer : m_swapchain_framebuffers)
    vkDestroyFramebuffer(m_device, framebuffer, NULL);
  return PAPER_kSuccess;
}

/* -------------------------------------------------------------------------- */
/*                                DEPTH STENCIL                               */
/* -------------------------------------------------------------------------- */

/**
 * @brief Creates the depth stencil for the render pass
 *
 * A depth buffer is an additional attachment that stores the depth for every
 * position, just like the color attachment stores the color of every position.
 * Every time the rasterizer produces a fragment, the depth test will check if
 * the new fragment is closer than the previous one. If it isn't, then the new
 * fragment is discarded. A fragment that passes the depth test writes its own
 * depth to the depth buffer.
 *
 * In this function, we allocate space in the device for a depth image to be
 * used for depth testing our swapchain images. The depth stencil attachment
 * and color attachment are the only attachments we need.
 *
 * Note that we only use a single depth stencil across all swapchain images.
 */
PAPER_TSuccess VKContext::createDepthStencil() {
  // create a single depth stencil image on the GPU
  VkImageCreateInfo imageCI{};
  imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCI.imageType = VK_IMAGE_TYPE_2D;
  imageCI.format = m_depth_format;
  imageCI.extent = {m_render_extent.width, m_render_extent.height, 1};
  imageCI.mipLevels = 1;
  imageCI.arrayLayers = 1;
  imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  VK_CHECK_RESULT(
      vkCreateImage(m_device, &imageCI, nullptr, &m_depth_stencil.image));

  // get the memory requirements of the image
  VkMemoryRequirements memReqs{};
  vkGetImageMemoryRequirements(m_device, m_depth_stencil.image, &memReqs);

  // allocate + bind memory for the image
  VkMemoryAllocateInfo memAllloc{};
  memAllloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAllloc.allocationSize = memReqs.size;
  memAllloc.memoryTypeIndex = m_vulkan_device->getMemoryType(
      memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  VK_CHECK_RESULT(
      vkAllocateMemory(m_device, &memAllloc, nullptr, &m_depth_stencil.mem));
  VK_CHECK_RESULT(vkBindImageMemory(m_device, m_depth_stencil.image,
                                    m_depth_stencil.mem, 0));

  // create the image view for the image
  VkImageViewCreateInfo imageViewCI{};
  imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
  imageViewCI.image = m_depth_stencil.image;
  imageViewCI.format = m_depth_format;
  imageViewCI.subresourceRange.baseMipLevel = 0;
  imageViewCI.subresourceRange.levelCount = 1;
  imageViewCI.subresourceRange.baseArrayLayer = 0;
  imageViewCI.subresourceRange.layerCount = 1;
  imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  // stencil aspect should only be set on depth + stencil formats
  // (VK_FORMAT_D16_UNORM_S8_UINT...VK_FORMAT_D32_SFLOAT_S8_UINT)
  if (m_depth_format >= VK_FORMAT_D16_UNORM_S8_UINT)
    imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
  VK_CHECK_RESULT(vkCreateImageView(m_device, &imageViewCI, nullptr,
                                    &m_depth_stencil.view));
}

/**
 * @brief Destroys the depth stencil image
 * @return PAPER_TSuccess Success always
 */
PAPER_TSuccess VKContext::destroyDepthStencil() {
  VK_SAFE_DELETE(m_depth_stencil.view,
                 vkDestroyImageView(m_device, m_depth_stencil.view, nullptr));
  VK_SAFE_DELETE(m_depth_stencil.image,
                 vkDestroyImage(m_device, m_depth_stencil.image, nullptr));
  VK_SAFE_DELETE(m_depth_stencil.mem,
                 vkFreeMemory(m_device, m_depth_stencil.mem, nullptr));
}

/* -------------------------------------------------------------------------- */
/*                                 RENDER PASS                                */
/* -------------------------------------------------------------------------- */

/**
 * @brief Creates the Vulkan render pass
 *
 * A render pass tells Vulkan about the framebuffer attachments that will be
 * used while rendering. We need to specify how many color and depth buffers
 * there will be, how many samples to use for each of them, and how their
 * contents should be handled throughout the rendering operations.
 *
 * @returns PAPER_TSuccess success always.
 */
PAPER_TSuccess VKContext::createRenderPass() {
  std::array<VkAttachmentDescription, 2> attachments = {};

  // define the color attachment
  attachments[0].format = m_swapchain.colorFormat;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  VkAttachmentReference colorReference = {};
  colorReference.attachment = 0;
  colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // define the depth attachment
  attachments[1].format = m_depth_format;
  attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  VkAttachmentReference depthReference = {};
  depthReference.attachment = 1;
  depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // create the subpass descriptor with color and depth attachments
  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorReference;
  subpass.pDepthStencilAttachment = &depthReference;
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = nullptr;
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments = nullptr;
  subpass.pResolveAttachments = nullptr;

  // subpass dependencies for layout transitions.
  // (COMPUTE) color -> depth stencil
  // (MEMORY) color -> depth stencil
  // color dependency
  std::array<VkSubpassDependency, 2> dependencies;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
  dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  // depth dependency
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
  dependencies[1].srcSubpass = 0;
  dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

  // finally, build the render pass with a single subpass, the one created above
  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
  renderPassInfo.pDependencies = dependencies.data();
  VK_CHECK_RESULT(
      vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_render_pass));
  return PAPER_kSuccess;
}

/* -------------------------------------------------------------------------- */
/*                                PIELINE CACHE                               */
/* -------------------------------------------------------------------------- */

/**
 * @brief Creates the Vulkan pipeline cache
 *
 * Pipeline cache objects allow the result of pipeline construction to be reused
 * between pipelines and between runs of an application. Reuse between pipelines
 * is achieved by passing the same pipeline cache object when creating multiple
 * related pipelines. Reuse across runs of an application is achieved by
 * retrieving pipeline cache contents in one run of an application, saving the
 * contents, and using them to preinitialize a pipeline cache on a subsequent
 * run.
 */
PAPER_TSuccess VKContext::createPipelineCache() {
  VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
  pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  VK_CHECK_RESULT(vkCreatePipelineCache(m_device, &pipelineCacheCreateInfo,
                                        nullptr, &m_pipeline_cache));
  return PAPER_kSuccess;
}

/* -------------------------------------------------------------------------- */
/*                                FRAME BUFFERS                               */
/* -------------------------------------------------------------------------- */

/**
 * @brief Creates the Vulkan framebuffers
 *
 * A framebuffer object references all of the VkImageView objects that represent
 * the attachments in a render subpass. As we're using both a color and a depth
 * attachment in our only subpass, we need to specify that we have 2 attachments
 * to our framebuffer create function. Then, we must create a framebuffer for
 * all images in the swap chain––as each will be different and require picking
 * the correct color image from the framebuffer.
 *
 * The depth image is shared across all framebuffers.
 */
PAPER_TSuccess VKContext::createFramebuffers() {
  VkImageView attachments[2];
  // depth stencil attachment is the same for all framebuffers
  attachments[1] = m_depth_stencil.view;
  // set common framebuffer information
  VkFramebufferCreateInfo fb_create_info = {};
  fb_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fb_create_info.pNext = NULL;
  fb_create_info.renderPass = m_render_pass;
  fb_create_info.attachmentCount = 2;
  fb_create_info.pAttachments = attachments;
  fb_create_info.width = m_render_extent.width;
  fb_create_info.height = m_render_extent.height;
  fb_create_info.layers = 1;

  // create the framebuffers
  m_swapchain_framebuffers.resize(m_swapchain.imageCount);
  for (uint32_t i = 0; i < m_swapchain.imageCount; i++) {
    attachments[0] = m_swapchain.buffers[i].view;
    // create the frame buffer with the swapchain image as an attachment
    VK_CHECK_RESULT(vkCreateFramebuffer(m_device, &fb_create_info, NULL,
                                        &m_swapchain_framebuffers[i]));
  }
}

/**
 * @brief Destroys the framebuffers (excluding those managed by the swapchain)
 *
 * @return PAPER_TSuccess Success always.
 */
PAPER_TSuccess VKContext::destroyFramebuffers() {
  for (auto framebuffer : m_swapchain_framebuffers)
    vkDestroyFramebuffer(m_device, framebuffer, NULL);
  return PAPER_kSuccess;
}

/* -------------------------------------------------------------------------- */
/*                               WINDOW RESIZING                              */
/* -------------------------------------------------------------------------- */
// We want to be able to support window resizing, which means we need to be
// capable of recreating our swap chain for the new dimensions (as well as all
// of the components that depend on the swap chain itself).

/**
 * @brief Runs a single frame of rendering
 *
 * Really, swaps an image that is ready to be presented in the swapchain to the
 * present surface, and frees up a stale image to be rendered to.
 *
 * @return PAPER_TSuccess
 */
PAPER_TSuccess VKContext::swapBuffers() {
  // wait until current frame is ready to present
  vkWaitForFences(m_device, 1, &m_in_flight_fences[m_currentFrame], VK_TRUE,
                  UINT64_MAX);

  // acquire the next image from the swap chain
  VkResult result = m_swapchain.acquireNextImage(
      m_image_available_semaphores[m_currentFrame], &m_currentImage);
  // if out-of-date, recreate the swap chain and skip this frame
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    recreateSwapchain();
    return PAPER_kFailure;
  } else if (result != VK_SUCCESS) {
    LOG("Error: Failed to acquire swap chain image. \n");
    return PAPER_kFailure;
  }

  // check if a previous frame is using this image (i.e. its fence to wait on)
  if (m_in_flight_images[m_currentImage] != VK_NULL_HANDLE)
    vkWaitForFences(m_device, 1, &m_in_flight_images[m_currentImage], VK_TRUE,
                    UINT64_MAX);
  m_in_flight_images[m_currentImage] = m_in_flight_fences[m_currentFrame];

  // submit command buffers to the graphics queue
  VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &m_image_available_semaphores[m_currentFrame];
  submit_info.pWaitDstStageMask = wait_stages;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &m_command_buffers[m_currentImage];
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &m_render_finished_semaphores[m_currentFrame];
  vkResetFences(m_device, 1, &m_in_flight_fences[m_currentFrame]);
  VK_CHECK_RESULT(vkQueueSubmit(m_queue, 1, &submit_info,
                                m_in_flight_fences[m_currentFrame]));
  do {
    result = vkWaitForFences(m_device, 1, &m_in_flight_fences[m_currentFrame],
                             VK_TRUE, 10000);
  } while (result == VK_TIMEOUT);
  VK_CHECK_RESULT(vkQueueWaitIdle(m_queue));

  // now present the finished image
  result = m_swapchain.queuePresent(
      m_queue, m_currentImage, m_render_finished_semaphores[m_currentFrame]);

  // check once more for an out-of-date swapchain
  // if out-of-date, recreate the swap chain and skip this frame
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    recreateSwapchain();
    return PAPER_kFailure;
  } else if (result != VK_SUCCESS) {
    LOG("Error: Failed to acquire swap chain image. \n");
    return PAPER_kFailure;
  }

  // increment the current frame and takes its modulo
  m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
  return PAPER_kSuccess;
}

/**
 * @brief Recreates the swapchain and its dependent resources
 *
 * Destroys + creates the framebuffers, render pass, depth stencil,
 * synchronization primitives, command buffers, and swap chain, and rebuilds
 * them.
 */
PAPER_TSuccess VKContext::recreateSwapchain() {
  // ensure all operations on the device have been finished before destruction
  vkDeviceWaitIdle(m_device);

  // destroy all components except for swapchain (handled in vks::VKSwapchain)
  destroyFramebuffers();
  vkDestroyRenderPass(m_device, m_render_pass, NULL);
  destroyDepthStencil();
  destroySynchronizationPrimitives();
  destroyCommandBuffers();

  // recreate the swap chain at new dimensions
  createSwapchain();

  // create the swap chain and successive components
  createCommandBuffers();
  createSynchronizationPrimitives();
  createDepthStencil();
  createRenderPass();
  createFramebuffers();

  // wait for idle again for next frame
  vkDeviceWaitIdle(m_device);
  return PAPER_kSuccess;
}

}  // namespace paperarium::gpu

/* -------------------------------------------------------------------------- */
/*                               STATIC HELPERS */
/* -------------------------------------------------------------------------- */
// Iterating over extensions and layers is annoying and takes up a lot of
// space, so their functions have been moved down here to organize the file.

/* --------------------------------- LAYERS --------------------------------- */

/**
 * @brief Get the layers available to Vulkan instances
 *
 * @return std::vector<VkLayerProperties>
 */
static std::vector<VkLayerProperties> layers_get_available() {
  uint32_t layer_count = 0;
  vkEnumerateInstanceLayerProperties(&layer_count, NULL);
  std::vector<VkLayerProperties> layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, layers.data());
  return layers;
}

/**
 * @brief Enables a layer for the Vulkan instance
 *
 * @return Indication if the layer is found & supported
 */
static bool layers_enable_layer(
    std::vector<VkLayerProperties>& layers_available,
    std::vector<char const*>& layers_enabled, char const* layer_name) {
  for (auto const& layer : layers_available) {
    if (strcmp(layer_name, layer.layerName) == 0) {
      layers_enabled.push_back(layer_name);
      return true;
    }
  }
  fprintf(stderr, "Error: Layer %s not supported.\n", layer_name);
  return false;
}

/* ------------------------------- EXTENSIONS ------------------------------- */

/**
 * @brief Get the extensions available to Vulkan instances
 *
 * @return std::vector<VkExtensionProperties>
 */
static std::vector<VkExtensionProperties> exts_get_available() {
  uint32_t ext_count = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);
  std::vector<VkExtensionProperties> extensions(ext_count);
  vkEnumerateInstanceExtensionProperties(nullptr, &ext_count,
                                         extensions.data());
  return extensions;
}

/**
 * @brief Enables an extension for the Vulkan instance
 *
 * @return Indication if the extension is found & supported
 */
static bool exts_enable_ext(
    std::vector<VkExtensionProperties>& extensions_available,
    std::vector<char const*>& extensions_enabled, char const* ext_name) {
  for (auto const& extension : extensions_available) {
    if (strcmp(ext_name, extension.extensionName) == 0) {
      extensions_enabled.push_back(ext_name);
      return true;
    }
  }
  fprintf(stderr, "Error: Extension %s not supported.\n", ext_name);
  return false;
}

/**
 * @brief Checks if a physical device supports an extension
 *
 * @return Indication if the extension is supported
 */
static bool exts_device_supports(VkPhysicalDevice device,
                                 std::vector<char const*>& extensions_enabled) {
  uint32_t ext_count;
  vkEnumerateDeviceExtensionProperties(device, NULL, &ext_count, NULL);
  std::vector<VkExtensionProperties> available_exts(ext_count);
  vkEnumerateDeviceExtensionProperties(device, NULL, &ext_count,
                                       available_exts.data());
  for (auto const& extension_needed : extensions_enabled) {
    bool found = false;
    for (auto const& extension : available_exts) {
      if (strcmp(extension_needed, extension.extensionName) == 0) {
        found = true;
        break;
      }
    }
    if (!found) return false;
  }
  return true;
}