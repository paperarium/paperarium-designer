/*
 * gpu_context_private.hh
 * Created by: Evan Kirkiles
 * Created on: Fri Dec 16 2022
 * From: source/blender/gpu/intern/gpu_context_private.hh
 * for Paperarium
 */

#ifndef GPU_CONTEXT_PRIVATE_HH
#define GPU_CONTEXT_PRIVATE_HH

#include "GPU_context.h"
#include "gpu_common.h"
#include "gpu_framebuffer_private.hh"
#include "gpu_matrix_private.hh"
#include <pthread.h>

namespace paperarium::gpu {

class Context {
 public:
  /** State management */
  Shader* shader = nullptr;
  Framebuffer* active_fb = nullptr;
  GPUMatrixState* matrix_state = nullptr;

  /**
   * All 4 window frame-buffers.
   * None of them are valid in an off-screen context.
   * Right frame-buffers are only available if using stereo rendering.
   * Front frame-buffers contains (in principle, but not always) the last frame
   * color. Default frame-buffer is back_left.
   */
  Framebuffer* back_left = nullptr;
  Framebuffer* front_left = nullptr;

  /* GPUContext counter used to assign a unique ID to each GPUContext. */
  static int context_counter;
  int context_id = 0;

 public:
  Context();
  virtual ~Context();

  static Context* get();

 protected:
  /** Thread on which this context is active. */
  pthread_t thread_;
  bool is_active_;
  /** Avoid including GHOST headers. Can be nullptr for off-screen contexts. */
  PLATF_SURF_MEMBERS

 public:
  virtual void activate() = 0;
  virtual void deactivate() = 0;
  virtual void begin_frame() = 0;
  virtual void end_frame() = 0;

  /* Will push all pending commands to the GPU. */
  virtual void flush() = 0;
  /* Will wait until the GPU has finished executing all command. */
  virtual void finish() = 0;

  bool is_active_on_thread();
};

/* Syntactic sugar. */
static inline GPUContext* wrap(Context* ctx) {
  return reinterpret_cast<GPUContext*>(ctx);
}
static inline Context* unwrap(GPUContext* ctx) {
  return reinterpret_cast<Context*>(ctx);
}
static inline Context const* unwrap(GPUContext const* ctx) {
  return reinterpret_cast<Context const*>(ctx);
}

}  // namespace paperarium::gpu

#endif /* GPU_CONTEXT_PRIVATE_HH */
