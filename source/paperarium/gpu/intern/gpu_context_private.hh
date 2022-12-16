/*
 * gpu_context_private.hh
 * Created by: Evan Kirkiles
 * Created on: Fri Dec 16 2022
 * for Paperarium
 */
#ifndef GPU_CONTEXT_PRIVATE_HH
#define GPU_CONTEXT_PRIVATE_HH

namespace paperarium::gpu {

class Context {
 public:
  /** State management */
  Shader* shader = nullptr;
  FrameBuffer* active_fb = nullptr;

 protected:
  /** Thread on which this context is active. */
  pthread_t thread_;
  bool is_active_;

 public:
  Context();
  virtual ~Context();
}

}  // namespace paperarium::gpu

#endif /* GPU_CONTEXT_PRIVATE_HH */
