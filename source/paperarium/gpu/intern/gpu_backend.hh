/*
 * gpu_backend.hh
 * Created by: Evan Kirkiles
 * Created on: Sun Dec 18 2022
 * From: source/blender/gpu/intern/gpu_backend.hh
 * for Paperarium
 */
#ifndef GPU_BACKEND_HH
#define GPU_BACKEND_HH

// #include "GPU_vertex_buffer.h"

namespace paperarium::gpu {

class Context;

class Batch;
class DrawList;
class Fence;
class FrameBuffer;
class IndexBuf;
class PixelBuffer;
class QueryPool;
class Shader;
class Texture;
class UniformBuf;
class StorageBuf;
class VertBuf;

/**
 * @brief Singleton GPU backend class
 *
 * Represents the interface through which to manipulate the GPU.
 */
class GPUBackend {
 public:
  virtual ~GPUBACKEND() = default;
  virtual void delete_resources() = 0;

  static GPUBackend* get();

  virtual Context* context_alloc() = 0;

  virtual Batch* batch_alloc() = 0;
  virtual Fence* fence_alloc() = 0;
  virtual FrameBuffer* framebuffer_alloc(char const* name) = 0;
  virtual IndexBuf* indexbuf_alloc() = 0;
  virtual PixelBuffer* pixelbuf_alloc(uint size) = 0;
  virtual QueryPool* querypool_alloc() = 0;
  virtual Shader* shader_alloc(char const* name) = 0;
  virtual Texture* texture_alloc(char const* name) = 0;
  virtual UniformBuf* uniformbuf_alloc(int size, char const* name) = 0;
  virtual StorageBuf* storagebuf_alloc(int size, GPUUsageType usage,
                                       char const* name) = 0;
  virtual VertBuf* vertbuf_alloc() = 0;

  /** Render frame coordination*/
  virtual void render_begin() = 0;
  virtual void render_end() = 0;
  virtual void render_step() = 0;
}

}  // namespace paperarium::gpu

#endif /* GPU_BACKEND_HH */
