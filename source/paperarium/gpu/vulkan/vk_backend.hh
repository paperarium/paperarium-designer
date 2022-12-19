/*
 * vk_backend.hh
 * Created by: Evan Kirkiles
 * Created on: Fri Dec 16 2022
 * for Paperarium
 */
#ifndef VK_BACKEND_HH
#define VK_BACKEND_HH

#include "gpu_backend.hh"

namespace paperarium::gpu {

class VKContext;

class VKBackend : public GPUBackend {
 public:
  VKBackend() { VKBackend::init_platform(); }
  virtual ~VKBackend() { VKBackend::platform_exit(); }

  void delete_resources() override;

  /**
   * @brief Initializes a VKContext for a surface
   */
  Context* context_alloc(
#if defined(VK_USE_PLATFORM_WIN32_KHR)
      HINSTANCE platformHandle, HWND platformWindow
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
      IDirectFB* dfb, IDirectFBSurface* window
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
      wl_display* display, wl_surface* window
#elif defined(VK_USE_PLATFORM_XCB_KHR)
      xcb_connection_t* connection, xcb_window_t window
#elif defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK)
      void* view
#elif defined(_DIRECT2DISPLAY) || defined(VK_USE_PLATFORM_HEADLESS_EXT)
      uint32_t width, uint32_t height
#endif
      ) override {
    return new VKContext(
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        platformHandle, platformWindow,
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
        dfb, window,
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
        display, window,
#elif defined(VK_USE_PLATFORM_XCB_KHR)
        connection, window,
#elif defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK)
        view,
#elif defined(_DIRECT2DISPLAY) || defined(VK_USE_PLATFORM_HEADLESS_EXT)
        width, height,
#endif
        true  // <--- DEBUG VALUE
    );
  }

  Batch* batch_alloc() override;
  DrawList* drawlist_alloc(int list_length) override;
  Fence* fence_alloc() override;
  FrameBuffer* framebuffer_alloc(char const* name) override;
  IndexBuf* indexbuf_alloc() override;
  PixelBuffer* pixelbuf_alloc(uint size) override;
  QueryPool* querypool_alloc() override;
  Shader* shader_alloc(char const* name) override;
  Texture* texture_alloc(char const* name) override;
  UniformBuf* uniformbuf_alloc(int size, char const* name) override;
  StorageBuf* storagebuf_alloc(int size, GPUUsageType usage,
                               char const* name) override;
  VertBuf* vertbuf_alloc() override;

  /* Render Frame Coordination --
   * Used for performing per-frame actions globally */
  void render_begin() override;
  void render_end() override;
  void render_step() override;

  static void capabilities_init(VKContext& context);

 private:
  static void platform_init();
  static void platform_exit();
}

}  // namespace paperarium::gpu

#endif /* VK_BACKEND_HH */
