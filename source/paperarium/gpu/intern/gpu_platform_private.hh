/*
 * gpu_platform_private.hh
 * Created by: Evan Kirkiles
 * Created on: Sun Dec 18 2022
 * From: source/blender/gpu/intern/gpu_platform_private.hh
 * for Paperarium
 */
#ifndef GPU_PLATFORM_PRIVATE_HH
#define GPU_PLATFORM_PRIVATE_HH

#include "GPU_platform.h"

namespace paperarium::gpu {

/**
 * @brief Contains information about the GPU platform being used
 *
 * Only a single GPU platform is put into effect at a time. This platform is
 * specified within GPG, for GPUPlatformGlobal.
 */
class GPUPlatformGlobal {
 public:
  bool initialized = false;
  eGPUDeviceType device;
  eGPUOSType os;
  eGPUDriverType driver;
  eGPUSupportLevel support_level;
  eGPUBackendType backend = GPU_BACKEND_NONE;
  char* vendor = nullptr;
  char* renderer = nullptr;
  char* version = nullptr;
  char* support_key = nullptr;
  char* gpu_name = nullptr;

 public:
  void init(eGPUDeviceType gpu_device, eGPUOSType os_type,
            eGPUDriverType driver_type, eGPUSupportLevel gpu_support_level,
            eGPUBackendType backend, char const* vendor_str,
            char const* renderer_str, char const* version_str);

  void clear();
};

// platform is defined application-wide!
extern GPUPlatformGlobal GPG;

}  // namespace paperarium::gpu

#endif /* GPU_PLATFORM_PRIVATE_HH */
