/*
 * vk_backend.cc
 * Created by: Evan Kirkiles
 * Created on: Fri Dec 16 2022
 * From: source/blender/gpu/opengl/gl_backend.cc
 * for Paperarium
 */

#include "vk_backend.hh"
#include "gpu_platform_private.hh"
#include "vk_initializers.hh"
#include <assert.h>
#include <vector>

namespace paperarium::gpu {

void VKBackend::platform_init() {
  // ensure the GPG has not been initialized
  assert(!GPG.initialized);

  // get the device properties, which include vendor information
  // ! This will not work. Platform_init must be called after vkcontext is made.
  auto device_properties = VKContext::get()->m_vulkan_device->m_properties;
  uint32_t apiVersion = device_properties.apiVersion;
  uint32_t driverVersion = device_properties.driverVersion;
  uint32_t vendorID = device_properties.vendorID;
  uint32_t deviceID = device_properties.deviceID;
  VkPhysicalDeviceType deviceType = device_properties.deviceType;
  char* deviceName = device_properties.deviceName;

  // these fields will be filled in
  eGPUDeviceType device = GPU_DEVICE_ANY;
  eGPUOSType os = GPU_OS_ANY;
  eGPUDriverType driver = GPU_DRIVER_ANY;
  eGPUSupportLevel support_level = GPU_SUPPORT_LEVEL_LIMITED;

#ifdef _WIN32
  os = GPU_OS_WIN;
#elif defined(__APPLE__)
  os = GPU_OS_MAC;
#else
  os = GPU_OS_UNIX;
#endif

  // parse known vendor IDs into fields
  if (!vendorID) {
    printf("Warning: No Vulkan vendor detected.\n");
    device = GPU_DEVICE_UNKNOWN;
    driver = GPU_DRIVER_ANY;
  } else if (vendorID == 0x1002) {  // AMD
    device = GPU_DEVICE_ATI;
    driver = GPU_DRIVER_OFFICIAL;
  } else if (vendorID == 0x10DE) {  // NVIDIA
    device = GPU_DEVICE_NVIDIA;
    driver = GPU_DRIVER_OFFICIAL;
  } else if (vendorID == 0x8086) {  // INTEL
    device = GPU_DEVICE_INTEL;
    driver = GPU_DRIVER_OFFICIAL;
  } else {
    printf(
        "Warning: Could not find a matching GPU name. Things may not behave as "
        "expected.\n");
    printf("Detected Vulkan configuration:\n");
    printf("Vendor: %08X\n", vendorID);
    printf("Device ID: %08X\n", deviceID);
  }

  // initialize GPG
  // GPG.init(device, os, driver, support_level, GPU_BACKEND_VULKAN, vendor,
  //          renderer, version);
}

/**
 * @brief Removes Vulkan information from the GPU Platform global
 */
void VKBackend::platform_exit() {
  assert(GPG.initialized);
  GPG.clear();
}

}  // namespace paperarium::gpu