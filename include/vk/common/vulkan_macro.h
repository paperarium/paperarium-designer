#ifndef VULKAN_MACRO_H
#define VULKAN_MACRO_H

#include "render_common.h"

// Safe deletion of Vulkan objects
#define VK_SAFE_DELETE(var, f)      \
{                                   \
    if (var != VK_NULL_HANDLE) {    \
        (f);                        \
        var = VK_NULL_HANDLE;       \
    }                               \
}

// Export wrappers
#ifdef VULKANENGINE_EXPORT
#ifdef WIN32
#define VULKANENGINE_EXPORT_API __declspec(dllexport)
#else
#define VULKANENGINE_EXPORT_API __attribute__((visibility("default")))
#endif
#else
#define VULKANENGINE_EXPORT_API
#endif

#endif // VULKAN_MACRO_H
