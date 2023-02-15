set(INC
  .
  intern
  vulkan
  ../draw
  ../makesdna)

set(INC_SYS
  ${GLM_INCLUDE_DIRS}
  ${Vulkan_INCLUDE_DIRS})

set(SRC
  intern/gpu_context.cc
  intern/gpu_framebuffer.cc
  intern/gpu_matrix.cc
  intern/gpu_platform.cc
  intern/gpu_texture.cc
  intern/gpu_viewport.c

  GPU_context.h
  GPU_framebuffer.h
  GPU_matrix.h
  GPU_platform.h
  GPU_texture.h
  GPU_viewport.h

  intern/gpu_backend.hh
  intern/gpu_common.h
  intern/gpu_context_private.hh
  intern/gpu_framebuffer_private.hh
  intern/gpu_matrix_private.hh
  intern/gpu_platform_private.hh
  intern/gpu_texture_private.hh
)

set(VULKAN_SRC
  vulkan/vk_backend.cc
  vulkan/vk_context.cc
  vulkan/vk_device.cc
  vulkan/vk_framebuffer.cc
  vulkan/vk_swapchain.cc
  vulkan/vk_texture.cc
  vulkan/vk_tools.cc

  vulkan/vk_backend.hh
  vulkan/vk_context.hh
  vulkan/vk_device.hh
  vulkan/vk_initializers.hh
  vulkan/vk_swapchain.hh
  vulkan/vk_texture_private.hh
  vulkan/vk_tools.h
)

set(LIB
  glm
  ${Vulkan_LIBRARIES}
)

list(APPEND SRC ${VULKAN_SRC})
list(APPEND INC ${GLM_INCLUDE_DIRS} ${VULKAN_INCLUDE_DIRS})
# list(APPEND INC ../../../extern/vulkan_memory_allocator)
# list(APPEND LIB extern_vulkan_memory_allocator)

# build library
add_library(pp_gpu ${SRC})
target_include_directories(pp_gpu PRIVATE ${INC})
target_include_directories(pp_gpu SYSTEM PRIVATE ${INC_SYS})
target_link_libraries(pp_gpu PUBLIC ${LIB})