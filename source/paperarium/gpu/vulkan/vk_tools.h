#ifndef VK_TOOLS_H
#define VK_TOOLS_H

#include <vulkan/vulkan.h>
#include <iostream>
#include <string>

// Safe deletion of Vulkan objects
#define VK_SAFE_DELETE(var, f)   \
  {                              \
    if (var != VK_NULL_HANDLE) { \
      (f);                       \
      var = VK_NULL_HANDLE;      \
    }                            \
  }

// Checking result of Vulkan return value
#define VK_CHECK_RESULT(f)                                                  \
  {                                                                         \
    VkResult res = (f);                                                     \
    if (res != VK_SUCCESS) {                                                \
      std::cout << "Fatal : VkResult is \"" << vks::tools::errorString(res) \
                << "\" in " << __FILE__ << " at line " << __LINE__          \
                << std::endl;                                               \
      assert(res == VK_SUCCESS);                                            \
    }                                                                       \
  }

// Logging
#define LOG(...) printf(__VA_ARGS__)
#define LOGI(...) printf(__VA_ARGS__)
#define DEBUG_PRINTF(...) \
  if (m_debug) printf(__VA_ARGS__);

// success / failure error code
typedef enum { PAPER_kFailure = 0, PAPER_kSuccess } PAPER_TSuccess;

// Checking result of Paper return value
#define PAPER_CHECK_RESULT(f)                                             \
  {                                                                       \
    PAPER_TSuccess res = (f);                                             \
    if (res != PAPER_kSuccess) {                                          \
      std::cout << "Fatal : PAPER_kSuccess is \""                         \
                << "FAILURE\" in " << __FILE__ << " at line " << __LINE__ \
                << std::endl;                                             \
      assert(res == PAPER_kSuccess);                                      \
    }                                                                     \
  }

// Custom define for better code readability
#define VK_FLAGS_NONE 0
// Default fence timeout in nanoseconds
#define DEFAULT_FENCE_TIMEOUT 100000000000

/* -------------------------------------------------------------------------- */
/*                                    TOOLS                                   */
/* -------------------------------------------------------------------------- */

namespace vks {
namespace tools {
/** @brief Disable message boxes on fatal errors */
extern bool errorModeSilent;

/** @brief Returns an error code as a string */
std::string errorString(VkResult errorCode);

/** @brief Returns the device type as a string */
std::string physicalDeviceTypeString(VkPhysicalDeviceType type);

// Selected a suitable supported depth format starting with 32 bit down to 16
// bit Returns false if none of the depth formats in the list is supported by
// the device
VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice,
                                 VkFormat* depthFormat);

// Put an image memory barrier for setting an image layout on the sub resource
// into the given command buffer
void setImageLayout(
    VkCommandBuffer cmdbuffer, VkImage image, VkImageLayout oldImageLayout,
    VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange,
    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
// Uses a fixed sub resource layout with first mip level and layer
void setImageLayout(
    VkCommandBuffer cmdbuffer, VkImage image, VkImageAspectFlags aspectMask,
    VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

/** @brief Inser an image memory barrier into the command buffer */
void insertImageMemoryBarrier(VkCommandBuffer cmdbuffer, VkImage image,
                              VkAccessFlags srcAccessMask,
                              VkAccessFlags dstAccessMask,
                              VkImageLayout oldImageLayout,
                              VkImageLayout newImageLayout,
                              VkPipelineStageFlags srcStageMask,
                              VkPipelineStageFlags dstStageMask,
                              VkImageSubresourceRange subresourceRange);

// Display error message and exit on fatal error
void exitFatal(std::string message, int32_t exitCode);
void exitFatal(std::string message, VkResult resultCode);

}  // namespace tools
}  // namespace vks

#endif /* VK_TOOLS_H */
