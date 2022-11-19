/*
 * VulkanBase.cpp
 * Created by: Evan Kirkiles
 * Created on: Fri Oct 28 2022
 * for Paperarium Design
 *
 * Much code written by wjl's:
 * https://github.com/engineer1109/LearnVulkan/blob/master/source/base/VulkanBase.cxx
 * And Sascha Willems:
 * https://github.com/SaschaWillems/Vulkan/blob/master/source/base/vulkanexamplebase.cpp
 */

#include "VulkanBase.h"

namespace VulkanEngine {

/* -------------------------------------------------------------------------- */
/*                            VULKAN INITIALIZATION                           */
/* -------------------------------------------------------------------------- */
// Process:
//   1. createInstance
//   2. pickPhysicalDevice
//   3. createLogicalDevice

/**
 * @brief Initializes the Vulkan instance
 *
 * Creates the instance and picks a physical device to use for Vulkan, reading
 * it as a logical device so we can easily access information about it.
 */
void VulkanBase::initVulkan() {
  createInstance();
  pickPhysicalDevice();
  createLogicalDevice();
}

/* ----------------------------- IMPLEMENTATION ----------------------------- */

/**
 * @brief Builds the Vulkan instance itself
 *
 * Applies platform-specific extensions as well as validation layers (if we
 * are using debug mode) that enable easy cross-platform Vulkan development.
 */
void VulkanBase::createInstance() {
  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Paperarium Design";
  appInfo.pEngineName = "Paperarium Design";
  appInfo.apiVersion = VK_API_VERSION_1_0;
  std::vector<char const*> instanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME};

  // enable surface extensions depending on OS
#if defined(_WIN32)
  instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(_DIRECT2DISPLAY)
  instanceExtensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
  instanceExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
  instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
  instanceExtensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_HEADLESS_EXT)
  instanceExtensions.push_back(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
#endif

  // get extensions supported by the instance and store for later use
  uint32_t extCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
  if (extCount > 0) {
    std::vector<VkExtensionProperties> extensions(extCount);
    if (vkEnumerateInstanceExtensionProperties(
            nullptr, &extCount, &extensions.front()) == VK_SUCCESS) {
      for (VkExtensionProperties extension : extensions) {
        m_supportedInstanceExtensions.push_back(extension.extensionName);
      }
    }
  }

  // SRS - When running on iOS/macOS with MoltenVK, enable
  // VK_KHR_get_physical_device_properties2 if not already enabled
  // (required by VK_KHR_portability_subset)
#if defined(VK_USE_PLATFORM_MACOS_MVK)
  if (std::find(m_enabledInstanceExtensions.begin(),
                m_enabledInstanceExtensions.end(),
                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) ==
      m_enabledInstanceExtensions.end()) {
    m_enabledInstanceExtensions.push_back(
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
  }
#endif

  // enable requested instance extensions
  if (m_enabledInstanceExtensions.size() > 0) {
    for (char const* enabledExtension : m_enabledInstanceExtensions) {
      instanceExtensions.push_back(enabledExtension);
    }
  }

  // build the Vulkan instance create struct
  VkInstanceCreateInfo instanceCreateInfo = {};
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pNext = NULL;
  instanceCreateInfo.pApplicationInfo = &appInfo;

#if defined(VK_USE_PLATFORM_MACOS_MVK)
  // SRS - When running on iOS/macOS with MoltenVK and
  // VK_KHR_portability_enumeration is defined and supported by the instance,
  // enable the extension and the flag
  if (std::find(m_supportedInstanceExtensions.begin(),
                m_supportedInstanceExtensions.end(),
                VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) !=
      m_supportedInstanceExtensions.end()) {
    instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
  }
#endif

  // add extensions
  if (instanceExtensions.size() > 0) {
    if (m_debug) {
      instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
      instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }
    instanceCreateInfo.enabledExtensionCount =
        (uint32_t)instanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
  }
  // if debugging, add validation layer
  if (m_debug) {
    // the VK_LAYER_KHRONOS_validation contains all current validation
    // functionality
    char const* validationLayerName = "VK_LAYER_KHRONOS_validation";
    // check if this layer is available at instance level
    uint32_t instanceLayerCount;
    vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
    std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
    vkEnumerateInstanceLayerProperties(&instanceLayerCount,
                                       instanceLayerProperties.data());
    bool validationLayerPresent = false;
    for (VkLayerProperties layer : instanceLayerProperties) {
      if (strcmp(layer.layerName, validationLayerName) == 0) {
        validationLayerPresent = true;
        break;
      }
    }
    // if the layer is available, add it to the instance
    if (validationLayerPresent) {
      instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
      instanceCreateInfo.enabledLayerCount = 1;
    } else {
      LOGI(
          "Validation layer VK_LAYER_KHRONOS_validation not present, "
          "validation is disabled");
    }
  }
  // finally, attempt to create the instance.
  m_result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);
  VK_CHECK_RESULT(m_result);
}

/**
 * @brief Selects a GPU to render the Vulkan instance with.
 *
 * Defaults to the first encountered device.
 */
void VulkanBase::pickPhysicalDevice() {
  uint32_t gpuCount = 0;
  VK_CHECK_RESULT(vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr));
  assert(gpuCount > 0);
  // enumerate Vulkan-capable devices
  std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
  m_result =
      vkEnumeratePhysicalDevices(m_instance, &gpuCount, physicalDevices.data());

  // GPU selection

  // select physical device to be used for the Vulkan instance.
  // defaults to the first device unless specified by command line.
  uint32_t selectedDevice = 0;
  m_physicalDevice = physicalDevices[selectedDevice];

  // store properties (including limits), features, and memory properties of
  // the physical device
  vkGetPhysicalDeviceProperties(m_physicalDevice, &m_deviceProperties);
  vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_deviceFeatures);
  vkGetPhysicalDeviceMemoryProperties(m_physicalDevice,
                                      &m_deviceMemoryProperties);

  // get extensions supported by the device and store for later use
  uint32_t extCount = 0;
  vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount,
                                       nullptr);
  if (extCount > 0) {
    std::vector<VkExtensionProperties> extensions(extCount);
    if (vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr,
                                             &extCount, &extensions.front()) ==
        VK_SUCCESS) {
      for (VkExtensionProperties extension : extensions) {
        m_supportedDeviceExtensions.push_back(extension.extensionName);
      }
    }
  }

#if defined(VK_USE_PLATFORM_MACOS_MVK)
  // SRS - When running on iOS/macOS with MoltenVK and VK_KHR_portability_subset
  // is defined and supported by the instance, enable the extension
  if (std::find(m_supportedDeviceExtensions.begin(),
                m_supportedDeviceExtensions.end(),
                "VK_KHR_portability_subset") !=
      m_supportedInstanceExtensions.end()) {
    m_enabledDeviceExtensions.push_back("VK_KHR_portability_subset");
  }
#endif

  // we can override actual features to enable for logical device creation,
  // if we want to do some testing.
  getDeviceFeatures();
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
void VulkanBase::createLogicalDevice() {
  // initialize a VulkanDevice from the physical device data
  m_vulkanDevice = new vks::VulkanDevice(m_physicalDevice);
  m_result = m_vulkanDevice->createLogicalDevice(
      m_enabledFeatures, m_enabledDeviceExtensions, m_deviceCreatepNextChain);
  m_device = m_vulkanDevice->logicalDevice;
  vkGetDeviceQueue(m_device, m_vulkanDevice->queueFamilyIndices.graphics, 0,
                   &m_queue);

  // find a suitable depth format
  VkBool32 validDepthFormat =
      vks::tools::getSupportedDepthFormat(m_physicalDevice, &m_depthFormat);
  assert(validDepthFormat);

  // link the Vulkan instance's swap chain from the logical to the physical
  // device
  m_swapChain.connect(m_instance, m_physicalDevice, m_device);

  // create synchronization objects
  VkSemaphoreCreateInfo semaphoreCreateInfo =
      vks::initializers::semaphoreCreateInfo();
  // create a semaphore used to synchronize image presentations
  // ensures that the image is displayed before we start submitting new commands
  // to the queue
  VK_CHECK_RESULT(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr,
                                    &m_semaphores.presentComplete));
  // create a semaphore used to synchronize command submission
  // ensures that the image is not presented until all commands have been
  // submitted and executed
  VK_CHECK_RESULT(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr,
                                    &m_semaphores.renderComplete));

  // set up submit info structure
  // semaphores will stay the same during application lifetime
  m_submitInfo = vks::initializers::submitInfo();
  m_submitInfo.pWaitDstStageMask = &m_submitPipelineStages;
  m_submitInfo.waitSemaphoreCount = 1;
  m_submitInfo.pWaitSemaphores = &m_semaphores.presentComplete;
  m_submitInfo.signalSemaphoreCount = 1;
  m_submitInfo.pSignalSemaphores = &m_semaphores.renderComplete;
}

/* -------------------------------------------------------------------------- */
/*                          VULKAN SWAPCHAIN CREATION                         */
/* -------------------------------------------------------------------------- */
// Process:
//  1.  prepareFunctions
//  2.  initSwapchain
//  3.  createCommandPool
//  4.  createSwapChain
//  5.  createCommandBuffers
//  6.  createSynchronizationPrimitives
//  7.  createDepthStencil
//  8.  createRenderPass
//  9.  createPipelineCache
//  10. createFramebuffers

/**
 * @brief Wrapper around the prepareBase function
 */
void VulkanBase::prepare() {
  prepareBase();
  m_prepared = true;
}

/**
 * @brief Prepares the base Vulkan engine for rendering.
 *
 * Initializes the swapchain, command pool, compmmand buffers, depth stencil,
 * render pass, pipeline cache, and frame buffer.
 */
void VulkanBase::prepareBase() {
  prepareFunctions();
  initSwapchain();
  createCommandPool();
  createSwapChain();
  createCommandBuffers();
  createSynchronizationPrimitives();
  createDepthStencil();
  createRenderPass();
  createPipelineCache();
  createFramebuffers();
}

/* ----------------------------- IMPLEMENTATIONS ---------------------------- */

/**
 * @brief Initializes the Vulkan swap chain wrapper
 *
 * This creates a link between the surface and our Vulkan logic. We can then
 * use the VulkanSwapchain wrapper to create a new swapchain, building upon
 * the previous one, without having to manage everything ourselves.
 */
void VulkanBase::initSwapchain() {
#if defined(_WIN32)
  m_swapChain.initSurface(m_windowInstance, m_window);
#elif (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
  m_swapChain.initSurface(reinterpret_cast<void*>(m_winId));
#elif defined(VK_USE_PLATFORM_XCB_KHR)
  m_swapChain.initSurface(m_connection, m_window);
#endif
}

/**
 * @brief Creates the Vulkan command pool
 *
 * A command pool manages the memory required to store buffers, and command
 * buffers are then allocated from them. The command pool can be initialized
 * with settings that tell the program about how often command buffers will be
 * rewritten to potentially optimize memory usage.
 */
void VulkanBase::createCommandPool() {
  VkCommandPoolCreateInfo cmdPoolInfo = {};
  cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cmdPoolInfo.queueFamilyIndex = m_swapChain.queueNodeIndex;
  // need this flag to be able to record a command buffer every frame
  cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  VK_CHECK_RESULT(
      vkCreateCommandPool(m_device, &cmdPoolInfo, nullptr, &m_cmdPool));
}

/**
 * @brief Creates the Vulkan swap chain
 *
 * A swapchain is simply a queue of images waiting to be shown––its general
 * purpose is to synchronize the presentation of images with the refresh rate of
 * the screen. All creation logic is handled for us by the VulkanSwapchain
 * ease-of-use wrapper.
 *
 * If a swap chain already exists for the surface, this function retires it.
 */
void VulkanBase::createSwapChain() {
  m_swapChain.create(&m_width, &m_height, false);
}

/**
 * @brief Creates the Vulkan command buffers
 *
 * Commands in Vulkan are not executed directly using functions, but are rather
 * pre-defined in command buffer objects to allow more deterministic behavior.
 * Here, we create a command buffer for each of our swap chain's images.
 */
void VulkanBase::createCommandBuffers() {
  m_drawCmdBuffers.resize(m_swapChain.imageCount);
  VkCommandBufferAllocateInfo cmdBufAllocateInfo =
      vks::initializers::commandBufferAllocateInfo(
          m_cmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY,
          static_cast<uint32_t>(m_drawCmdBuffers.size()));
  // perform the command buffer allocation
  VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &cmdBufAllocateInfo,
                                           m_drawCmdBuffers.data()));
}

/**
 * @brief Creates the Vulkan fences
 *
 * Fences are used in Vulkan to synchronize code across the CPU and GPU. We
 * implement a fence for each command buffer, and thus for each swap chain
 * image. They serve to inform the CPU when the GPU has finished drawing each
 * image, i.e. when an image in the swapchain is ready to be displayed.
 */
void VulkanBase::createSynchronizationPrimitives() {
  VkFenceCreateInfo fenceCreateInfo =
      vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
  m_waitFences.resize(m_drawCmdBuffers.size());
  for (auto& fence : m_waitFences) {
    VK_CHECK_RESULT(vkCreateFence(m_device, &fenceCreateInfo, nullptr, &fence));
  }
}

/**
 * @brief Creates the Vulkan depth stencil for the render pass
 *
 * A depth buffer is an additional attachment that stores the depth for every
 * position, just like the color attachment stores the color of every position.
 * Every time the rasterizer produces a fragment, the depth test will check if
 * the new fragment is closer than the previous one. If it isn't, then the new
 * fragment is discarded. A fragment that passes the depth test writes its own
 * depth to the depth buffer.
 *
 * In this function, we allocate space in the device for a depth image to be
 * used for depth testing our swapchain images.
 */
void VulkanBase::createDepthStencil() {
  VkImageCreateInfo imageCI{};
  imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCI.imageType = VK_IMAGE_TYPE_2D;
  imageCI.format = m_depthFormat;
  imageCI.extent = {m_width, m_height, 1};
  imageCI.mipLevels = 1;
  imageCI.arrayLayers = 1;
  imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  VK_CHECK_RESULT(
      vkCreateImage(m_device, &imageCI, nullptr, &m_depthStencil.image));

  VkMemoryRequirements memReqs{};
  vkGetImageMemoryRequirements(m_device, m_depthStencil.image, &memReqs);

  VkMemoryAllocateInfo memAllloc{};
  memAllloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAllloc.allocationSize = memReqs.size;
  memAllloc.memoryTypeIndex = m_vulkanDevice->getMemoryType(
      memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  VK_CHECK_RESULT(
      vkAllocateMemory(m_device, &memAllloc, nullptr, &m_depthStencil.mem));
  VK_CHECK_RESULT(
      vkBindImageMemory(m_device, m_depthStencil.image, m_depthStencil.mem, 0));

  VkImageViewCreateInfo imageViewCI{};
  imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
  imageViewCI.image = m_depthStencil.image;
  imageViewCI.format = m_depthFormat;
  imageViewCI.subresourceRange.baseMipLevel = 0;
  imageViewCI.subresourceRange.levelCount = 1;
  imageViewCI.subresourceRange.baseArrayLayer = 0;
  imageViewCI.subresourceRange.layerCount = 1;
  imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  // stencil aspect should only be set on depth + stencil formats
  // (VK_FORMAT_D16_UNORM_S8_UINT...VK_FORMAT_D32_SFLOAT_S8_UINT)
  if (m_depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT)
    imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
  VK_CHECK_RESULT(
      vkCreateImageView(m_device, &imageViewCI, nullptr, &m_depthStencil.view));
}

/**
 * @brief Creates the Vulkan render pass
 *
 * A render pass tells Vulkan about the framebuffer attachments that will be
 * used while rendering. We need to specify how many color and depth buffers
 * there will be, how many samples to use for each of them, and how their
 * contents should be handled throughout the rendering operations.
 */
void VulkanBase::createRenderPass() {
  std::array<VkAttachmentDescription, 2> attachments = {};

  // color attachment
  attachments[0].format = m_swapChain.colorFormat;
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

  // depth attachment
  attachments[1].format = m_depthFormat;
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

  // create our subpass with color and depth attachments
  VkSubpassDescription subpassDescription = {};
  subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescription.colorAttachmentCount = 1;
  subpassDescription.pColorAttachments = &colorReference;
  subpassDescription.pDepthStencilAttachment = &depthReference;
  subpassDescription.inputAttachmentCount = 0;
  subpassDescription.pInputAttachments = nullptr;
  subpassDescription.preserveAttachmentCount = 0;
  subpassDescription.pPreserveAttachments = nullptr;
  subpassDescription.pResolveAttachments = nullptr;

  // Subpass dependencies for layout transitions
  std::array<VkSubpassDependency, 2> dependencies;
  dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
  dependencies[1].srcSubpass = 0;
  dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  // finally build the render pass with a single subpass, the one created above
  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpassDescription;
  renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
  renderPassInfo.pDependencies = dependencies.data();
  VK_CHECK_RESULT(
      vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass));
}

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
void VulkanBase::createPipelineCache() {
  VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
  pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  VK_CHECK_RESULT(vkCreatePipelineCache(m_device, &pipelineCacheCreateInfo,
                                        nullptr, &m_pipelineCache));
}

/**
 * @brief Creates the Vulkan framebuffers
 *
 * A framebuffer object references all of the VkImageView objects that represent
 * the attachments in a render subpass. As we're using both a color and a depth
 * attachment in our only subpass, we need to specify that we have 2 attachments
 * to our framebuffer create function. Then, we must create a framebuffer for
 * all images in the swap chain––as each will be different and require picking
 * the correct color/depth image from the framebuffer.
 */
void VulkanBase::createFramebuffers() {
  VkImageView attachments[2];

  // Depth/stencil attachment is the same for all frame buffers
  attachments[1] = m_depthStencil.view;

  VkFramebufferCreateInfo frameBufferCreateInfo = {};
  frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  frameBufferCreateInfo.pNext = NULL;
  frameBufferCreateInfo.renderPass = m_renderPass;
  frameBufferCreateInfo.attachmentCount = 2;
  frameBufferCreateInfo.pAttachments = attachments;
  frameBufferCreateInfo.width = m_width;
  frameBufferCreateInfo.height = m_height;
  frameBufferCreateInfo.layers = 1;

  // Create frame buffers for every wap chain image
  m_frameBuffers.resize(m_swapChain.imageCount);
  for (uint32_t i = 0; i < m_frameBuffers.size(); i++) {
    attachments[0] = m_swapChain.buffers[i].view;
    VK_CHECK_RESULT(vkCreateFramebuffer(m_device, &frameBufferCreateInfo,
                                        nullptr, &m_frameBuffers[i]));
  }
}

/* -------------------------------------------------------------------------- */
/*                             VULKAN DESTRUCTION                             */
/* -------------------------------------------------------------------------- */
// Some components of our Vulkan instance need to be manually cleaned up by us
// when we destroy the Vulkan instance.

/**
 * @brief Destroy the Vulkan Base::VulkanBase object
 *
 * Performs safe deletes on all Vulkan engine resources to safely quit from the
 * Vulkan instance.
 */
VulkanBase::~VulkanBase() {
  VK_CHECK_RESULT(vkQueueWaitIdle(m_queue));
  vkDeviceWaitIdle(m_device);
  destroySurface();
  VK_SAFE_DELETE(m_descriptorPool,
                 vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr));
  VK_SAFE_DELETE(m_renderPass,
                 vkDestroyRenderPass(m_device, m_renderPass, nullptr));
  for (auto& shaderModule : m_shaderModules)
    VK_SAFE_DELETE(shaderModule,
                   vkDestroyShaderModule(m_device, shaderModule, nullptr));
  VK_SAFE_DELETE(m_pipelineCache,
                 vkDestroyPipelineCache(m_device, m_pipelineCache, nullptr));
  VK_SAFE_DELETE(
      m_semaphores.presentComplete,
      vkDestroySemaphore(m_device, m_semaphores.presentComplete, nullptr));
  VK_SAFE_DELETE(
      m_semaphores.renderComplete,
      vkDestroySemaphore(m_device, m_semaphores.renderComplete, nullptr));
  for (auto& fence : m_waitFences)
    VK_SAFE_DELETE(fence, vkDestroyFence(m_device, fence, nullptr));
  VK_SAFE_DELETE(m_cmdPool, vkDestroyCommandPool(m_device, m_cmdPool, nullptr));
  delete_ptr(m_vulkanDevice);
  VK_SAFE_DELETE(m_instance, vkDestroyInstance(m_instance, nullptr));
}

/* ----------------------------- IMPLEMENTATION ----------------------------- */

/**
 * @brief Destroys the swap chain and all its dependencies
 *
 * This fully destroys the swap chain, removing its link to the surface we
 * defined before. Therefore, we only use this when we want to entirely clean up
 * our Vulkan instance. Otherwise, you are OK just calling
 * m_swapChain.create(...) to recreate the swapChain. We also delete the depth
 * stencil attachment, the command buffers, and all of the frame buffers.
 */
void VulkanBase::destroySurface() {
  if (!m_prepared) return;
  m_swapChain.cleanup();
  destroyCommandBuffers();
  VK_SAFE_DELETE(m_depthStencil.view,
                 vkDestroyImageView(m_device, m_depthStencil.view, nullptr));
  VK_SAFE_DELETE(m_depthStencil.image,
                 vkDestroyImage(m_device, m_depthStencil.image, nullptr));
  VK_SAFE_DELETE(m_depthStencil.mem,
                 vkFreeMemory(m_device, m_depthStencil.mem, nullptr));
  for (uint32_t i = 0; i < m_frameBuffers.size(); i++)
    VK_SAFE_DELETE(m_frameBuffers[i],
                   vkDestroyFramebuffer(m_device, m_frameBuffers[i], nullptr));
  m_prepared = false;
}

/**
 * @brief Destroys the command buffers
 *
 * Frees the command buffers holding the commands for each of our swap chain's
 * images. Resets the size of m_drawCmdBuffers to 0.
 */
void VulkanBase::destroyCommandBuffers() {
  if (m_drawCmdBuffers.size() > 0)
    vkFreeCommandBuffers(m_device, m_cmdPool,
                         static_cast<uint32_t>(m_drawCmdBuffers.size()),
                         m_drawCmdBuffers.data());
  m_drawCmdBuffers.resize(0);
}

/* -------------------------------------------------------------------------- */
/*                        VULKAN SWAP CHAIN RECREATION                        */
/* -------------------------------------------------------------------------- */
// We want to be able to support window resizing, which means we need to be
// capable of recreating our swap chain for the new dimensions (as well as all
// of the components that depend on the swap chain itself).

/**
 * @brief Handles swap chain recreation on window resize
 *
 * First, waits for the device to enter an idle state. Then, rebuilds the swap
 * chain, recreates the attachments and frame buffers, and then recreates
 * the command buffers. Idles once prepared.
 */
void VulkanBase::windowResize() {
  if (!m_prepared) return;
  m_prepared = false;

  // ensure all operations on the device have been finished before destroying
  // resources
  vkDeviceWaitIdle(m_device);

  // recreate the swap chain
  m_width = m_destWidth;
  m_height = m_destHeight;
  createSwapChain();

  // recreate the frame buffers
  vkDestroyImageView(m_device, m_depthStencil.view, nullptr);
  vkDestroyImage(m_device, m_depthStencil.image, nullptr);
  vkFreeMemory(m_device, m_depthStencil.mem, nullptr);
  createDepthStencil();
  for (uint32_t i = 0; i < m_frameBuffers.size(); i++)
    vkDestroyFramebuffer(m_device, m_frameBuffers[i], nullptr);
  createFramebuffers();

  // command buffers need to be recreated as they may store references to the
  // recreated frame buffer
  destroyCommandBuffers();
  createCommandBuffers();
  buildCommandBuffers();

  // recreate fences in case number of swapchain images has changed on resize
  for (auto& fence : m_waitFences) vkDestroyFence(m_device, fence, nullptr);
  createSynchronizationPrimitives();

  vkDeviceWaitIdle(m_device);
  m_prepared = true;
}

/* -------------------------------------------------------------------------- */
/*                              VULKAN RENDERING                              */
/* -------------------------------------------------------------------------- */
// Our actual render logic follows, in which we prepare a frame, submit a
// command buffer to it to begin processing, and then submit the frame itself to
// the surface.
// !TODO: Move events over to QEvents

/**
 * @brief Calls the render function until the Vulkan instance is quit
 *
 * Continually polls for quit events while rendering frames and updating the
 * overlay. Once a quit event is received, vkDeviceWaitIdle.
 */
void VulkanBase::renderLoop() {
  while (!m_quit) {
    renderFrame();
    updateOverlay();
    m_scroll = 0.f;
  }
  // once we have quit, just idle the Vulkan instance
  if (m_device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(m_device);
  }
}

/**
 * @brief Renders a single frame to the device
 *
 * Calls render() and draw(), and then updates the Vulkan state based on
 * commands. Measures frame render timing and stores frame times in
 * m_frameTimer.
 */
void VulkanBase::renderFrame() {
  render();
  prepareFrame();
  m_submitInfo.commandBufferCount = 1;
  m_submitInfo.pCommandBuffers = &m_drawCmdBuffers[m_currentBuffer];
  VK_CHECK_RESULT(vkQueueSubmit(m_queue, 1, &m_submitInfo, VK_NULL_HANDLE));
  submitFrame();
  updateCommand();
  // now wait for queue to become idle
  VK_CHECK_RESULT(vkQueueWaitIdle(m_queue));
}

/* ----------------------------- IMPLEMENTATION ----------------------------- */

/**
 * @brief Base Vulkan render function (OVERRIDE THIS).
 */
void VulkanBase::render() {}

/**
 * @brief Draws the rendered frame to the surface
 *
 * Prepares a frame to acquire the next image from the swap chain. Then submits
 * the command buffers which define our color and depth attachments, telling
 * how to draw the image acquired from the swap chain. Finally, submits
 * the frame once the command buffers have been run, drawing the frame to
 * the surface!
 */
void VulkanBase::draw() {
  if (m_stop || m_pause) return;
  m_signalFrame = false;
  prepareFrame();

  // command buffer to be submitted to the queue
  m_submitInfo.commandBufferCount = 1;
  m_submitInfo.pCommandBuffers = &m_drawCmdBuffers[m_currentBuffer];

  // now submit to ithe queue
  if (m_prepared)
    VK_CHECK_RESULT(vkQueueSubmit(m_queue, 1, &m_submitInfo, VK_NULL_HANDLE));

  submitFrame();
  m_signalFrame = true;
}

/* --------------------------- DEEP IMPLEMENTATION -------------------------- */

/**
 * @brief Acquires the next image in the Vulkan swap chain.
 *
 * If the swap chain is no longer compatible with the surface (resized), we also
 * handle recreation here, as m_swapChain.acquireNextImage will error out in
 * that case. Otherwise, we are ready to submit the image from the swap chain
 * to the surface.
 */
void VulkanBase::prepareFrame() {
  if (m_pause || !m_prepared) return;
  // acquire the next image from the swap chain
  VkResult err = m_swapChain.acquireNextImage(m_semaphores.presentComplete,
                                              &m_currentBuffer);
  // recreate the swapchain if it's no longer compatible with the surface
  // (OUT_OF_DATE) or no longer optimal for presentation (SUBOPTIMAL)
  if ((err == VK_ERROR_OUT_OF_DATE_KHR) || (err = VK_SUBOPTIMAL_KHR)) {
    if (err == VK_ERROR_OUT_OF_DATE_KHR) windowResize();
    return;
  } else {
    VK_CHECK_RESULT(err);
  }
}

/**
 * @brief Submits an image from the Vulkan swap chain to the surface
 *
 * This function actually presents the swap chain's image to the configured
 * surface. We also listen for window resize events here. This function
 * is predicated on the render complete semaphore, which lets us know when
 * the image is completed and ready to show.
 */
void VulkanBase::submitFrame() {
  VkResult err = m_swapChain.queuePresent(m_queue, m_currentBuffer,
                                          m_semaphores.renderComplete);
  // recreate the swapchain if it's no longer compatible with the surface
  // (OUT_OF_DATE) or no longer optimal for presentation (SUBOPTIMAL)
  if ((err == VK_ERROR_OUT_OF_DATE_KHR) || (err = VK_SUBOPTIMAL_KHR)) {
    windowResize();
    if (err == VK_ERROR_OUT_OF_DATE_KHR) return;
  } else {
    VK_CHECK_RESULT(err);
  }
}

/* -------------------------------------------------------------------------- */
/*                                   HELPERS                                  */
/* -------------------------------------------------------------------------- */

/**
 * @brief Handles mouse movement. I think I don't need this.
 *
 * @param x
 * @param y
 */
void VulkanBase::handleMouseMove(float x, float y) {
  m_mousePos = glm::vec2(x, y);
}

/**
 * @brief Runs a saved Vulkan function
 *
 * @param i
 */
void VulkanBase::runFunction(int i) {
  if (i < m_functions.size()) {
    m_functions[i]();
  }
}

/**
 * @brief Waits for the current frame to be rendered.
 *
 * Sleeps the thread until the frame has been drawn. This frees up resources for
 * other processes, preventing the thread from blocking.
 */
void VulkanBase::waitForCurrentFrameComplete() {
  m_pause = true;
  while (m_signalFrame == false) {
    std::this_thread::sleep_for(std::chrono::microseconds(1));
  }
}

}  // namespace VulkanEngine
