/*
 * vk_framebuffer.cc
 * Created by: Evan Kirkiles
 * Created on: Tue Dec 20 2022
 * for Paperarium
 * From: source/blender/gpu/opengl/gl_framebuffer.cc
 */
#include "vk_framebuffer.hh"
#include "vk_context.hh"

namespace paperarium::gpu {

VKFramebuffer::VKFramebuffer(char const* name) : Framebuffer(name) {}
VKFramebuffer::~VKFramebuffer() {
  if (m_context == nullptr) return;
}

/**
 * @brief Initializes the framebuffer
 *
 * Does not create the framebuffer on the GPU, as we need the attachments ready
 * to be able to do that.
 */
void VKFramebuffer::init() { context_ = VKContext::get(); }

void VKFramebuffer::update_attachments() {
  /* First color texture OR the depth texture if no color is attached.
   * Used to determine frame-buffer color-space and dimensions. */
  GPUAttachmentType first_attachment = GPU_FB_MAX_ATTACHMENT;
  for (GPUAttachmentType type = GPU_FB_MAX_ATTACHMENT - 1; type >= 0; --type) {
    GPUAttachment& attach = attachments_[type];
    VkImageUsageFlagBits = to_vk(type);
    if (type >= GPU_FB_COLOR_ATTACHMENT0) {
      vk_attachments_[type - GPU_FB_COLOR_ATTACHMENT0] =
          (attach.tex) ? gl_attachment:
    }
  }
}

}  // namespace paperarium::gpu