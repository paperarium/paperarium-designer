#ifndef VK_TOOLS_H
#define VK_TOOLS_H

// Safe deletion of Vulkan objects
#define VK_SAFE_DELETE(var, f)   \
  {                              \
    if (var != VK_NULL_HANDLE) { \
      (f);                       \
      var = VK_NULL_HANDLE;      \
    }                            \
  }

// Logging
#define LOG(...) printf(__VA_ARGS__)
#define LOGI(...) printf(__VA_ARGS__)
#define DEBUG_PRINTF(...) \
  if (m_debug) printf(__VA_ARGS__);

// success / failure error code
typedef enum { PAPER_kFailure = 0, PAPER_kSuccess } PAPER_TSuccess;

// Custom define for better code readability
#define VK_FLAGS_NONE 0
// Default fence timeout in nanoseconds
#define DEFAULT_FENCE_TIMEOUT 100000000000

#endif /* VK_TOOLS_H */
