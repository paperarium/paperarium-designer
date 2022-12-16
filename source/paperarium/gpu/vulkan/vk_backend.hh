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
  static void init_platform();
  static void platform_exit();
}

}  // namespace paperarium::gpu

#endif /* VK_BACKEND_HH */
