/*
 * gpu_platform.cc
 * Created by: Evan Kirkiles
 * Created on: Sun Dec 18 2022
 * From: source/blender/gpu/intern/gpu_platform.cc
 * for Paperarium
 */

#include "GPU_platform.h"
#include "gpu_platform_private.hh"
#include <string>

namespace paperarium::gpu {

// the global GPU platform instance
GPUPlatformGlobal GPG;

static char* create_gpu_name(char const* vendor, char const* renderer,
                             char const* version) {
  DynStr* ds = BLI_dynstr_new();
  BLI_dynstr_appendf(ds, "%s %s %s", vendor, renderer, version);
  char* gpu_name = BLI_dynstr_get_cstring(ds);
  BLI_dynstr_free(ds);
  BLI_str_replace_char(gpu_name, '\n', ' ');
  BLI_str_replace_char(gpu_name, '\r', ' ');
  return gpu_name;
}

/* -------------------------------------------------------------------------- */
/*                              GPUPlatformGlobal                             */
/* -------------------------------------------------------------------------- */

/**
 * @brief Creates the global GPU platform identifier
 *
 * GPG is the global GPU platform, which identifies the GPU being used on the
 * backend and what graphics library is controlling it (GL, VK, etc.). GPG is
 * set up by a GPU_Backend upon initialization, and is most important for
 * allowing other parts of the app to know when a GPU Backend exists.
 *
 * @param gpu_device
 * @param os_type
 * @param driver_type
 * @param gpu_support_level
 * @param backend
 * @param vendor_str
 * @param renderer_str
 * @param version_str
 */
void GPUPlatformGlobal::init(eGPUDeviceType gpu_device, eGPUOSType os_type,
                             eGPUDriverType driver_type,
                             eGPUSupportLevel gpu_support_level,
                             eGPUBackendType backend, char const* vendor_str,
                             char const* renderer_str,
                             char const* version_str) {
  this->clear();
  this->initialized = true;
  this->device = gpu_device;
  this->os = os_type;
  this->driver = driver_type;
  this->support_level = gpu_support_level;

  char const* vendor = vendor_str ? vendor_str : "UNKNOWN";
  char const* renderer = renderer_str ? renderer_str : "UNKNOWN";
  char const* version = version_str ? version_str : "UNKNOWN";

  // this->vendor = BLI_strdup(vendor);
  // this->renderer = BLI_strdup(renderer);
  // this->version = BLI_strdup(version);
  // this->support_key = create_key(gpu_support_level, vendor, renderer,
  // version); this->gpu_name = create_gpu_name(vendor, renderer, version);
  this->backend = backend;
}

}  // namespace paperarium::gpu