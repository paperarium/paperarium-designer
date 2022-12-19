/*
 * vk_device.cc
 * Created by: Evan Kirkiles
 * Created on: Sat Dec 17 2022
 * for Paperarium
 */

#include "vk_device.hh"
#include "vk_macros.h"
#include <algorithm>
#include <assert.h>
#include <exception>

namespace vks {

VKDevice::VKDevice(VkPhysicalDevice physicalDevice) {
  assert(physicalDevice);
  m_physical_device = physicalDevice;

  // Store Properties features, limits and properties of the physical device
  // for later use Device properties also contain limits and sparse properties
  vkGetPhysicalDeviceProperties(physicalDevice, &m_properties);
  // Features should be checked by the examples before using them
  vkGetPhysicalDeviceFeatures(physicalDevice, &m_features);
  // Memory properties are used regularly for creating all kinds of buffers
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &m_memoryProperties);
  // Queue family properties, used for setting up requested queues upon device
  // creation
  uint32_t queueFamilyCount;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           nullptr);
  assert(queueFamilyCount > 0);
  m_queue_family_properties.resize(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           m_queue_family_properties.data());

  // Get list of supported extensions
  uint32_t extCount = 0;
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount,
                                       nullptr);
  if (extCount > 0) {
    std::vector<VkExtensionProperties> extensions(extCount);
    if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount,
                                             &extensions.front()) ==
        VK_SUCCESS) {
      for (auto ext : extensions) {
        m_supported_extensions.push_back(ext.extensionName);
      }
    }
  }
}

/**
 * Default destructor
 *
 * @note Frees the logical device
 */
VKDevice::~VKDevice() {
  VK_SAFE_DELETE(m_command_pool,
                 vkDestroyCommandPool(m_logical_device, m_command_pool, NULL));
  VK_SAFE_DELETE(m_logical_device, vkDestroyDevice(m_logical_device, NULL));
}

/* -------------------------------------------------------------------------- */
/*                              RESOURCE CREATION                             */
/* -------------------------------------------------------------------------- */

/**
 * Create the logical device based on the assigned physical device, also gets
 * default queue family indices
 *
 * @param enabledFeatures Can be used to enable certain features upon device
 * creation
 * @param pNextChain Optional chain of pointer to extension structures
 * @param useSwapChain Set to false for headless rendering to omit the
 * swapchain device extensions
 * @param requestedQueueTypes Bit flags specifying the queue types to be
 * requested from the device
 *
 * @return VkResult of the device creation call
 */
VkResult VKDevice::createLogicalDevice(
    VkPhysicalDeviceFeatures enabledFeatures,
    std::vector<char const*> enabledExtensions, void* pNextChain,
    bool useSwapChain, VkQueueFlags requestedQueueTypes) {
  // Desired queues need to be requested upon logical device creation
  // Due to differing queue family configurations of Vulkan implementations
  // this can be a bit tricky, especially if the application requests
  // different queue types

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

  // Get queue family indices for the requested queue family types
  // Note that the indices may overlap depending on the implementation
  float const defaultQueuePriority(0.0f);

  // Graphics queue
  if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
    m_queue_family_indices.graphics =
        getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = m_queue_family_indices.graphics;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &defaultQueuePriority;
    queueCreateInfos.push_back(queueInfo);
  } else {
    m_queue_family_indices.graphics = 0;
  }

  // Dedicated compute queue
  if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {
    m_queue_family_indices.compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
    if (m_queue_family_indices.compute != m_queue_family_indices.graphics) {
      // If compute family index differs, we need an additional queue create
      // info for the compute queue
      VkDeviceQueueCreateInfo queueInfo{};
      queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueInfo.queueFamilyIndex = m_queue_family_indices.compute;
      queueInfo.queueCount = 1;
      queueInfo.pQueuePriorities = &defaultQueuePriority;
      queueCreateInfos.push_back(queueInfo);
    }
  } else {
    // Else we use the same queue
    m_queue_family_indices.compute = queueFamilyIndices.graphics;
  }

  // Dedicated transfer queue
  if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT) {
    m_queue_family_indices.transfer =
        getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
    if ((m_queue_family_indices.transfer != m_queue_family_indices.graphics) &&
        (m_queue_family_indices.transfer != m_queue_family_indices.compute)) {
      // If compute family index differs, we need an additional queue create
      // info for the compute queue
      VkDeviceQueueCreateInfo queueInfo{};
      queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueInfo.queueFamilyIndex = m_queue_family_indices.transfer;
      queueInfo.queueCount = 1;
      queueInfo.pQueuePriorities = &defaultQueuePriority;
      queueCreateInfos.push_back(queueInfo);
    }
  } else {
    // Else we use the same queue
    m_queue_family_indices.transfer = m_queue_family_indices.graphics;
  }

  // Create the logical device representation
  std::vector<char const*> deviceExtensions(enabledExtensions);
  if (useSwapChain) {
    // If the device will be used for presenting to a display via a swapchain
    // we need to request the swapchain extension
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  }

  VkDeviceCreateInfo deviceCreateInfo = {};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());
  ;
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
  deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

  // If a pNext(Chain) has been passed, we need to add it to the device
  // creation info
  VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
  if (pNextChain) {
    physicalDeviceFeatures2.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physicalDeviceFeatures2.features = enabledFeatures;
    physicalDeviceFeatures2.pNext = pNextChain;
    deviceCreateInfo.pEnabledFeatures = nullptr;
    deviceCreateInfo.pNext = &physicalDeviceFeatures2;
  }

  // Enable the debug marker extension if it is present (likely meaning a
  // debugging tool is present)
  if (extensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
    deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    enableDebugMarkers = true;
  }

  if (deviceExtensions.size() > 0) {
    deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
  }

  VkResult result = vkCreateDevice(m_physical_device, &deviceCreateInfo,
                                   nullptr, &m_logical_device);

  if (result == VK_SUCCESS) {
    // Create a default command pool for graphics command buffers
    m_command_pool = createCommandPool(m_queue_family_indices.graphics);
  }

  m_enabled_features = enabledFeatures;

  return result;
}

/**
 * Create a buffer on the device
 *
 * @param usageFlags Usage flag bitmask for the buffer (i.e. index, vertex,
 * uniform buffer)
 * @param memoryPropertyFlags Memory properties for this buffer (i.e. device
 * local, host visible, coherent)
 * @param size Size of the buffer in byes
 * @param buffer Pointer to the buffer handle acquired by the function
 * @param memory Pointer to the memory handle acquired by the function
 * @param data Pointer to the data that should be copied to the buffer after
 * creation (optional, if not set, no data is copied over)
 *
 * @return VK_SUCCESS if buffer handle and memory have been created and
 * (optionally passed) data has been copied
 */
VkResult VKDevice::createBuffer(VkBufferUsageFlags usageFlags,
                                VkMemoryPropertyFlags memoryPropertyFlags,
                                VkDeviceSize size, VkBuffer* buffer,
                                VkDeviceMemory* memory, void* data = nullptr) {
  // Create the buffer handle
  VkBufferCreateInfo bufferCreateInfo =
      vks::initializers::bufferCreateInfo(usageFlags, size);
  bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  VK_CHECK_RESULT(
      vkCreateBuffer(m_logical_device, &bufferCreateInfo, nullptr, buffer));

  // Create the memory backing up the buffer handle
  VkMemoryRequirements memReqs;
  VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
  vkGetBufferMemoryRequirements(m_logical_device, *buffer, &memReqs);
  memAlloc.allocationSize = memReqs.size;
  // Find a memory type index that fits the properties of the buffer
  memAlloc.memoryTypeIndex =
      getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
  VK_CHECK_RESULT(
      vkAllocateMemory(m_logical_device, &memAlloc, nullptr, memory));

  // If a pointer to the buffer data has been passed, map the buffer and copy
  // over the data
  if (data != nullptr) {
    void* mapped;
    VK_CHECK_RESULT(
        vkMapMemory(m_logical_device, *memory, 0, size, 0, &mapped));
    memcpy(mapped, data, size);
    // If host coherency hasn't been requested, do a manual flush to make
    // writes visible
    if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
      VkMappedMemoryRange mappedRange = vks::initializers::mappedMemoryRange();
      mappedRange.memory = *memory;
      mappedRange.offset = 0;
      mappedRange.size = size;
      vkFlushMappedMemoryRanges(m_logical_device, 1, &mappedRange);
    }
    vkUnmapMemory(m_logical_device, *memory);
  }

  // Attach the memory to the buffer object
  VK_CHECK_RESULT(vkBindBufferMemory(m_logical_device, *buffer, *memory, 0));
  return VK_SUCCESS;
}

/**
 * Create a buffer on the device
 *
 * @param usageFlags Usage flag bitmask for the buffer (i.e. index, vertex,
 * uniform buffer)
 * @param memoryPropertyFlags Memory properties for this buffer (i.e. device
 * local, host visible, coherent)
 * @param buffer Pointer to a vk::Vulkan buffer object
 * @param size Size of the buffer in byes
 * @param data Pointer to the data that should be copied to the buffer after
 * creation (optional, if not set, no data is copied over)
 *
 * @return VK_SUCCESS if buffer handle and memory have been created and
 * (optionally passed) data has been copied
 */
VkResult VKDevice::createBuffer(VkBufferUsageFlags usageFlags,
                                VkMemoryPropertyFlags memoryPropertyFlags,
                                vks::Buffer* buffer, VkDeviceSize size,
                                void* data) {
  buffer->device = m_logicalDevice;

  // Create the buffer handle
  VkBufferCreateInfo bufferCreateInfo =
      vks::initializers::bufferCreateInfo(usageFlags, size);
  VK_CHECK_RESULT(vkCreateBuffer(m_logical_device, &bufferCreateInfo, nullptr,
                                 &buffer->buffer));

  // Create the memory backing up the buffer handle
  VkMemoryRequirements memReqs;
  VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
  vkGetBufferMemoryRequirements(m_logical_device, buffer->buffer, &memReqs);
  memAlloc.allocationSize = memReqs.size;
  // Find a memory type index that fits the properties of the buffer
  memAlloc.memoryTypeIndex =
      getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
  VK_CHECK_RESULT(
      vkAllocateMemory(m_logical_device, &memAlloc, nullptr, &buffer->memory));

  buffer->alignment = memReqs.alignment;
  buffer->size = size;
  buffer->usageFlags = usageFlags;
  buffer->memoryPropertyFlags = memoryPropertyFlags;

  // If a pointer to the buffer data has been passed, map the buffer and copy
  // over the data
  if (data != nullptr) {
    VK_CHECK_RESULT(buffer->map());
    memcpy(buffer->mapped, data, size);
    if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
      buffer->flush();

    buffer->unmap();
  }

  // Initialize a default descriptor that covers the whole buffer size
  buffer->setupDescriptor();

  // Attach the memory to the buffer object
  return buffer->bind();
}

/**
 * Copy buffer data from src to dst using VkCmdCopyBuffer
 *
 * @param src Pointer to the source buffer to copy from
 * @param dst Pointer to the destination buffer to copy tp
 * @param queue Pointer
 * @param copyRegion (Optional) Pointer to a copy region, if NULL, the whole
 * buffer is copied
 *
 * @note Source and destionation pointers must have the approriate transfer
 * usage flags set (TRANSFER_SRC / TRANSFER_DST)
 */
void VKDevice::copyBuffer(vks::Buffer* src, vks::Buffer* dst, VkQueue queue,
                          VkBufferCopy* copyRegion) {
  assert(dst->size <= src->size);
  assert(src->buffer);
  VKCommandBuffer copyCmd;
  createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, &copyCmd, 1, true);
  VkBufferCopy bufferCopy{};
  if (copyRegion == nullptr) {
    bufferCopy.size = src->size;
  } else {
    bufferCopy = *copyRegion;
  }
  vkCmdCopyBuffer(copyCmd, src->buffer, dst->buffer, 1, &bufferCopy);
  flushCommandBuffer(copyCmd, queue);
}

/**
 * Create a command pool for allocation command buffers from
 *
 * @param queueFamilyIndex Family index of the queue to create the command
 * pool for
 * @param createFlags (Optional) Command pool creation flags (Defaults to
 * VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
 *
 * @note Command buffers allocated from the created pool can only be submitted
 * to a queue with the same family index
 *
 * @return A handle to the created command buffer
 */
VkCommandPool VKDevice::createCommandPool(
    uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags) {
  VkCommandPoolCreateInfo cmdPoolInfo = {};
  cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
  cmdPoolInfo.flags = createFlags;
  VkCommandPool cmdPool;
  VK_CHECK_RESULT(
      vkCreateCommandPool(m_logical_device, &cmdPoolInfo, nullptr, &cmdPool));
  return cmdPool;
}

/**
 * Allocate a command buffer from the command pool
 *
 * @param level Level of the new command buffer (primary or secondary)
 * @param pool Command pool from which the command buffer will be allocated
 * @param (Optional) begin If true, recording on the new command buffer will
 * be started (vkBeginCommandBuffer) (Defaults to false)
 *
 * @return A handle to the allocated command buffer
 */
void VKDevice::createCommandBuffer(VkCommandBufferLevel level,
                                   VkCommandPool pool,
                                   VKCommandBuffer* pBuffers,
                                   uint32_t commandBufferCount, bool begin) {
  VkCommandBufferAllocateInfo cmdBufAllocateInfo =
      vks::initializers::commandBufferAllocateInfo(pool, level,
                                                   commandBufferCount);
  VK_CHECK_RESULT(vkAllocateCommandBuffers(m_logical_device,
                                           &cmdBufAllocateInfo, pBuffers));
  // If requested, also start recording for the new command buffers
  if (begin) {
    VkCommandBufferBeginInfo cmdBufInfo =
        vks::initializers::commandBufferBeginInfo();
    for (int i = 0; i < commandBufferCount; i++) {
      VK_CHECK_RESULT(vkBeginCommandBuffer(pBuffers[i], &cmdBufInfo));
    }
  }
  return cmdBuffer;
}

/**
 * Finish command buffer recording and submit it to a queue
 *
 * @param commandBuffer Command buffer to flush
 * @param queue Queue to submit the command buffer to
 * @param pool Command pool on which the command buffer has been created
 * @param free (Optional) Free the command buffer once it has been submitted
 * (Defaults to true)
 *
 * @note The queue that the command buffer is submitted to must be from the
 * same family index as the pool it was allocated from
 * @note Uses a fence to ensure command buffer has finished executing
 */
void VKDevice::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue,
                                  VkCommandPool pool, bool free) {
  if (commandBuffer == VK_NULL_HANDLE) return;
  VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

  VkSubmitInfo submitInfo = vks::initializers::submitInfo();
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
  // Create fence to ensure that the command buffer has finished executing
  VkFenceCreateInfo fenceInfo =
      vks::initializers::fenceCreateInfo(VK_FLAGS_NONE);
  VkFence fence;
  VK_CHECK_RESULT(vkCreateFence(m_logical_device, &fenceInfo, nullptr, &fence));
  // Submit to the queue
  VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
  // Wait for the fence to signal that command buffer has finished executing
  VK_CHECK_RESULT(vkWaitForFences(m_logical_device, 1, &fence, VK_TRUE,
                                  DEFAULT_FENCE_TIMEOUT));
  vkDestroyFence(m_logical_device, fence, nullptr);
  if (free) vkFreeCommandBuffers(m_logical_device, pool, 1, &commandBuffer);
}

/* -------------------------------------------------------------------------- */
/*                               DEVICE QUERIES                               */
/* -------------------------------------------------------------------------- */

/**
 * Get the index of a memory type that has all the requested property bits
 * set
 *
 * @param typeBits Bitmask with bits set for each memory type supported by
 * the resource to request for (from VkMemoryRequirements)
 * @param properties Bitmask of properties for the memory type to request
 * @param (Optional) memTypeFound Pointer to a bool that is set to true if a
 * matching memory type has been found
 *
 * @return Index of the requested memory type
 *
 * @throw Throws an exception if memTypeFound is null and no memory type
 * could be found that supports the requested properties
 */
uint32_t VKDevice::getMemoryType(uint32_t typeBits,
                                 VkMemoryPropertyFlags properties,
                                 VkBool32* memTypeFound = nullptr) {
  for (uint32_t i = 0; i < m_memory_properties.memoryTypeCount; i++) {
    if ((typeBits & 1) == 1) {
      if ((m_memory_properties.memoryTypes[i].propertyFlags & properties) ==
          properties) {
        if (memTypeFound) {
          *memTypeFound = true;
        }
        return i;
      }
    }
    typeBits >>= 1;
  }

  if (memTypeFound) {
    *memTypeFound = false;
    return 0;
  } else {
    throw std::runtime_error("Could not find a matching memory type");
  }
}

/**
 * Get the index of a queue family that supports the requested queue flags
 *
 * @param queueFlags Queue flags to find a queue family index for
 *
 * @return Index of the queue family index that matches the flags
 *
 * @throw Throws an exception if no queue family index could be found that
 * supports the requested flags
 */
uint32_t VKDevice::getQueueFamilyIndex(VkQueueFlagBits queueFlags) {
  // Dedicated queue for compute
  // Try to find a queue family index that supports compute but not graphics
  if (queueFlags & VK_QUEUE_COMPUTE_BIT) {
    for (uint32_t i = 0;
         i < static_cast<uint32_t>(m_queue_family_properties.size()); i++) {
      if ((m_queue_family_properties[i].queueFlags & queueFlags) &&
          ((m_queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) ==
           0)) {
        return i;
        break;
      }
    }
  }

  // Dedicated queue for transfer
  // Try to find a queue family index that supports transfer but not graphics
  // and compute
  if (queueFlags & VK_QUEUE_TRANSFER_BIT) {
    for (uint32_t i = 0;
         i < static_cast<uint32_t>(m_queue_family_properties.size()); i++) {
      if ((m_queue_family_properties[i].queueFlags & queueFlags) &&
          ((m_queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) ==
           0) &&
          ((m_queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) ==
           0)) {
        return i;
        break;
      }
    }
  }

  // For other queue types or if no separate compute queue is present, return
  // the first one to support the requested flags
  for (uint32_t i = 0;
       i < static_cast<uint32_t>(m_queue_family_properties.size()); i++) {
    if (m_queue_family_properties[i].queueFlags & queueFlags) {
      return i;
      break;
    }
  }

  throw std::runtime_error("Could not find a matching queue family index");
}

/**
 * Select the best-fit depth format for this device from a list of possible
 * depth (and stencil) formats
 *
 * @param checkSamplingSupport Check if the format can be sampled from (e.g.
 * for shader reads)
 *
 * @return The depth format that best fits for the current device
 *
 * @throw Throws an exception if no depth format fits the requirements
 */
VkFormat VKDevice::getSupportedDepthFormat(bool checkSamplingSupport) {
  // All depth formats may be optional, so we need to find a suitable depth
  // format to use
  std::vector<VkFormat> depthFormats = {
      VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT,
      VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT,
      VK_FORMAT_D16_UNORM};
  for (auto& format : depthFormats) {
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(m_physical_device, format,
                                        &formatProperties);
    // Format must support depth stencil attachment for optimal tiling
    if (formatProperties.optimalTilingFeatures &
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
      if (checkSamplingSupport) {
        if (!(formatProperties.optimalTilingFeatures &
              VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
          continue;
        }
      }
      return format;
    }
  }
  throw std::runtime_error("Could not find a matching depth format");
}

}  // namespace vks