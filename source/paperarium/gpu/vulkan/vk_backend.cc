/*
 * vk_backend.cc
 * Created by: Evan Kirkiles
 * Created on: Fri Dec 16 2022
 * for Paperarium
 */

#include "vk_backend.hh"

namespace paperarium::gpu {

void VKBackend::init_platform() {
  eGPUDeviceType device = GPU_DEVICE_ANY;
  eGPUOSType os = GPU_OS_ANY;
  eGPUDriverType driver = GPU_DRIVER_ANY;
  eGPUSupportLevel support_level = GPU_SUPPORT_LEVEL_SUPPORTED;
#ifdef _WIN32
  os = GPU_OS_WIN;
#elif defined(__APPLE__)
  os = GPU_OS_MAC;
#else
  os = GPU_OS_UNIX;
#endif
}

}  // namespace paperarium::gpu