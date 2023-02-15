/*
 * gpu_framebuffer_private.hh
 * Created by: Evan Kirkiles
 * Created on: Tue Dec 20 2022
 * for Paperarium
 * From: source/blender/gpu/intern/gpu_framebuffer_private.hh
 */
#ifndef GPU_FRAMEBUFFER_PRIVATE_HH
#define GPU_FRAMEBUFFER_PRIVATE_HH

#include "GPU_framebuffer.h"

struct GPUTexture;

typedef enum GPUAttachmentType : int {
  GPU_FB_DEPTH_ATTACHMENT = 0,
  GPU_FB_DEPTH_STENCIL_ATTACHMENT,
  GPU_FB_COLOR_ATTACHMENT0,
  GPU_FB_COLOR_ATTACHMENT1,
  GPU_FB_COLOR_ATTACHMENT2,
  GPU_FB_COLOR_ATTACHMENT3,
  GPU_FB_COLOR_ATTACHMENT4,
  GPU_FB_COLOR_ATTACHMENT5,
  GPU_FB_COLOR_ATTACHMENT6,
  GPU_FB_COLOR_ATTACHMENT7,
  /* Number of maximum output slots. */
  /* Keep in mind that GL max is GL_MAX_DRAW_BUFFERS and is at least 8,
   * corresponding to the maximum number of COLOR attachments specified by
   * glDrawBuffers. */
  GPU_FB_MAX_ATTACHMENT,
} GPUAttachmentType;

#define GPU_FB_MAX_COLOR_ATTACHMENT \
  (GPU_FB_MAX_ATTACHMENT - GPU_FB_COLOR_ATTACHMENT0)

namespace paperarium::gpu {

#ifdef DEBUG
#define DEBUG_NAME_LEN 64
#else
#define DEBUG_NAME_LEN 16
#endif

/**
 * @brief A framebuffer containing a set of attachments
 *
 * A framebuffer represents a view of a frame. These function as the read/write
 * destinations for shaders, being bound with their attachments to
 * different slots in the shader code.
 */
class Framebuffer {
 protected:
  /** Set of texture attachments to render to. DEPTH and DEPTH_STENCIL are
   * mutually exclusive. */
  GPUAttachment attachments_[GPU_FB_MAX_ATTACHMENT];
  /** Is true if internal representation need to be updated. */
  bool dirty_attachments_ = true;
  /** Size of attachment textures. */
  int width_ = 0, height_ = 0;
  /** Debug name. */
  char name_[DEBUG_NAME_LEN];
  /** Frame-buffer state. */
  int viewport_[4] = {0};
  int scissor_[4] = {0};
  bool scissor_test_ = false;
  bool dirty_state_ = true;

  /** Copy of the GL state. Contains ONLY color attachments enums for slot
   * binding. */
  // struct vk_attachment_ {
  //   VkImageView view;
  //   VkImage image;
  //   VkDeviceMemory memory;
  // }* vk_attachments_[GPU_FB_MAX_COLOR_ATTACHMENT];

 public:
  Framebuffer(char const* name);
  virtual ~Framebuffer();

  // attachment management
  void attachment_set(GPUAttachmentType type,
                      GPUAttachment const& new_attachment);
  void attachment_remove(GPUAttachmentType type);
};

/* Syntactic sugar. */
static inline GPUFramebuffer* wrap(Framebuffer* vert) {
  return reinterpret_cast<GPUFramebuffer*>(vert);
}
static inline Framebuffer* unwrap(GPUFramebuffer* vert) {
  return reinterpret_cast<Framebuffer*>(vert);
}
static inline Framebuffer const* unwrap(GPUFramebuffer const* vert) {
  return reinterpret_cast<Framebuffer const*>(vert);
}

}  // namespace paperarium::gpu

#endif /* GPU_FRAMEBUFFER_PRIVATE_HH */
