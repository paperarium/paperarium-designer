/*
 * vk_device.hh
 * Created by: Evan Kirkiles
 * Created on: Sat Dec 17 2022
 * for Paperarium
 *
 * Largely taken from Sascha Willem's VulkanDevice.h
 */
#ifndef VK_DEVICE_HH
#define VK_DEVICE_HH

#include "vulkan/vulkan.h"
#include <algorithm>
#include <string>
#include <vector>

namespace paperarium::gpu {

class VKDevice {
 public:
  /**  @brief Typecast to VkDevice */
  operator VkDevice() { return m_logical_device; }

  /**
   * @brief Constructs a logical representation of a physical Vulkan Device
   *
   * Populates the instance with information about the supported properties,
   * features, and extensions in the device.
   *
   * @param physicalDevice The physical device used as backing
   */
  VKDevice(VkPhysicalDevice physicalDevice);

  /**
   * @brief Destroy the VKDevice object
   *
   */
  ~VKDevice();

  /* --------------------------- RESOURCE CREATION -------------------------- */

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
  VkResult createLogicalDevice(
      VkPhysicalDeviceFeatures enabledFeatures,
      std::vector<char const*> enabledExtensions, void* pNextChain,
      bool useSwapChain = true,
      VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT |
                                         VK_QUEUE_COMPUTE_BIT);

  // /**
  //  * Create a buffer on the device
  //  *
  //  * @param usageFlags Usage flag bitmask for the buffer (i.e. index, vertex,
  //  * uniform buffer)
  //  * @param memoryPropertyFlags Memory properties for this buffer (i.e.
  //  device
  //  * local, host visible, coherent)
  //  * @param size Size of the buffer in byes
  //  * @param buffer Pointer to the buffer handle acquired by the function
  //  * @param memory Pointer to the memory handle acquired by the function
  //  * @param data Pointer to the data that should be copied to the buffer
  //  after
  //  * creation (optional, if not set, no data is copied over)
  //  *
  //  * @return VK_SUCCESS if buffer handle and memory have been created and
  //  * (optionally passed) data has been copied
  //  */
  // VkResult createBuffer(VkBufferUsageFlags usageFlags,
  //                       VkMemoryPropertyFlags memoryPropertyFlags,
  //                       VkDeviceSize size, VkBuffer* buffer,
  //                       VkDeviceMemory* memory, void* data = nullptr);

  // /**
  //  * Create a buffer on the device
  //  *
  //  * @param usageFlags Usage flag bitmask for the buffer (i.e. index, vertex,
  //  * uniform buffer)
  //  * @param memoryPropertyFlags Memory properties for this buffer (i.e.
  //  device
  //  * local, host visible, coherent)
  //  * @param buffer Pointer to a vk::Vulkan buffer object
  //  * @param size Size of the buffer in byes
  //  * @param data Pointer to the data that should be copied to the buffer
  //  after
  //  * creation (optional, if not set, no data is copied over)
  //  *
  //  * @return VK_SUCCESS if buffer handle and memory have been created and
  //  * (optionally passed) data has been copied
  //  */
  // VkResult createBuffer(VkBufferUsageFlags usageFlags,
  //                       VkMemoryPropertyFlags memoryPropertyFlags,
  //                       vks::Buffer* buffer, VkDeviceSize size,
  //                       void* data = nullptr);

  // /**
  //  * Copy buffer data from src to dst using VkCmdCopyBuffer
  //  *
  //  * @param src Pointer to the source buffer to copy from
  //  * @param dst Pointer to the destination buffer to copy tp
  //  * @param queue Pointer
  //  * @param copyRegion (Optional) Pointer to a copy region, if NULL, the
  //  whole
  //  * buffer is copied
  //  *
  //  * @note Source and destionation pointers must have the approriate transfer
  //  * usage flags set (TRANSFER_SRC / TRANSFER_DST)
  //  */
  // void copyBuffer(vks::Buffer* src, vks::Buffer* dst, VkQueue queue,
  //                 VkBufferCopy* copyRegion = nullptr);

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
  VkCommandPool createCommandPool(
      uint32_t queueFamilyIndex,
      VkCommandPoolCreateFlags createFlags =
          VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

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
  void createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool,
                           VkCommandBuffer* pBuffers,
                           uint32_t commandBufferCount = 1, bool begin = false);
  void createCommandBuffer(VkCommandBufferLevel level,
                           VkCommandBuffer* pBuffers,
                           uint32_t commandBufferCount = 1,
                           bool begin = false) {
    createCommandBuffer(level, m_command_pool, pBuffers, commandBufferCount,
                        begin);
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
  void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue,
                          VkCommandPool pool, bool free = true);
  void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue,
                          bool free = true) {
    return flushCommandBuffer(commandBuffer, queue, m_command_pool, free);
  }

  /* ---------------------------- DEVICE QUERIES ---------------------------- */

  /**
   * Get the index of a memory type that has all the requested property bits
   * set
   *
   * @param typeBits Bitmask with bits set for each memory type supported by
   * the resource to request for (from VkMemoryRequirements)
   * @param properties Bitmask of properties for the memory type to request
   * @param (Optional) memTypeFound Pointer to a bool that is set to true if
   * a matching memory type has been found
   *
   * @return Index of the requested memory type
   *
   * @throw Throws an exception if memTypeFound is null and no memory type
   * could be found that supports the requested properties
   */
  uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties,
                         VkBool32* memTypeFound = nullptr);

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
  uint32_t getQueueFamilyIndex(VkQueueFlagBits queueFlags);

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
  VkFormat getSupportedDepthFormat(bool checkSamplingSupport);

  /**
   * Check if an extension is supported by the (physical device)
   *
   * @param extension Name of the extension to check
   *
   * @return True if the extension is supported (present in the list read at
   * device creation time)
   */
  bool extensionSupported(std::string extension) {
    return (std::find(m_supported_extensions.begin(),
                      m_supported_extensions.end(),
                      extension) != m_supported_extensions.end());
  }

  /** @brief Properties of the physical device including limits that the
   * application can check against */
  VkPhysicalDeviceProperties m_properties;

 private:
  /** @brief Physical device representation */
  VkPhysicalDevice m_physical_device;
  /** @brief Logical device representation (application's view of the device) */
  VkDevice m_logical_device;
  /** @brief Features of the physical device that an application can use to
   * check if a feature is supported */
  VkPhysicalDeviceFeatures m_features;
  /** @brief Features that have been enabled for use on the physical device */
  VkPhysicalDeviceFeatures m_enabled_features;
  /** @brief Memory types and heaps of the physical device */
  VkPhysicalDeviceMemoryProperties m_memory_properties;
  /** @brief Queue family properties of the physical device */
  std::vector<VkQueueFamilyProperties> m_queue_family_properties;
  /** @brief List of extensions supported by the device */
  std::vector<std::string> m_supported_extensions;

  /** @brief Default command pool for the graphics queue family index */
  VkCommandPool m_command_pool = VK_NULL_HANDLE;

  /** @brief Set to true when the debug marker extension is detected */
  bool m_enable_debug_markers = false;

  /** @brief Contains queue family indices */
  struct {
    uint32_t graphics;
    uint32_t compute;
    uint32_t transfer;
  } m_queue_family_indices;
};

}  // namespace paperarium::gpu

#endif /* VK_DEVICE_HH */
