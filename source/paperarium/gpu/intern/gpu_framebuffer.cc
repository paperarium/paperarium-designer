/*
 * gpu_framebuffer.cc
 * Created by: Evan Kirkiles
 * Created on: Tue Dec 20 2022
 * for Paperarium
 * From: source/blender/gpu/intern/gpu_framebuffer.cc
 */

#include "GPU_framebuffer.h"
#include "gpu_framebuffer_private.hh"
#include "gpu_texture_private.hh"
#include <string>

namespace paperarium::gpu {

Framebuffer::Framebuffer(char const* name) {
  if (name) {
    strncpy(name_, name, sizeof(name_));
  } else {
    name_[0] = '\0';
  }
  /* Force config on first use. */
  dirty_attachments_ = true;
  dirty_state_ = true;

  // ensure all potential attachments are not initialized
  for (GPUAttachment& attachment : attachments_) {
    attachment.tex = nullptr;
    attachment.mip = -1;
    attachment.layer = -1;
  }
}

/**
 * @brief Destroy the Framebuffer:: Framebuffer object
 * Detaches all attachment images.
 */
Framebuffer::~Framebuffer() {
  for (GPUAttachment& attachment : attachments_) {
    if (attachment.tex != nullptr) {
      reinterpret_cast<Texture*>(attachment.tex)->detach_from(this);
    }
  }
}

/* -------------------------------------------------------------------------- */
/*                            ATTACHMENT MANAGEMENT                           */
/* -------------------------------------------------------------------------- */

/**
 * @brief Adds a new attachment to a framebuffer
 *
 * Attachments are essentially render targets in a render pass, being either
 * intermediary outputs between render passes or the final swapchain image.
 *
 * @param type
 * @param new_attachment
 */
void Framebuffer::attachment_set(GPUAttachmentType type,
                                 GPUAttachment const& new_attachment) {
  // ensure GPU attachment type is valid
  if (type >= GPU_FB_MAX_ATTACHMENT) {
    printf(
        "GPUFramebuffer: Error: Trying to attach texture to type %d but "
        "maximum slot is %d.\n",
        type - GPU_FB_COLOR_ATTACHMENT0, GPU_FB_MAX_COLOR_ATTACHMENT);
    return;
  }

  // make sure type and new_attachment are in agreement
  if (new_attachment.tex) {
    if (new_attachment.layer > 0) {
      assert(GPU_texture_cube(new_attachment.tex) ||
             GPU_texture_array(new_attachment.tex));
    }
    if (GPU_texture_stencil(new_attachment.tex)) {
      assert(type == GPU_FB_DEPTH_STENCIL_ATTACHMENT);
    } else if (GPU_texture_depth(new_attachment.tex)) {
      assert(type == GPU_FB_DEPTH_ATTACHMENT);
    }
  }

  // get the current attachment, if it's different
  GPUAttachment& attachment = attachments_[type];
  if (attachment.tex == new_attachment.tex) return;
  // now unbind the previous
  if (attachment.tex)
    reinterpret_cast<Texture*>(attachment.tex)->detach_from(this);
  attachment = new_attachment;
  // and bind the new one
  if (attachment.tex)
    reinterpret_cast<Texture*>(attachment.tex)->attach_to(this, type);

  // mark that we need to rebuild the framebuffer
  dirty_attachments_ = true;
}

void Framebuffer::attachment_remove(GPUAttachmentType type) {
  attachments_[type] = GPU_ATTACHMENT_NONE;
  dirty_attachments_ = true;
}

}  // namespace paperarium::gpu