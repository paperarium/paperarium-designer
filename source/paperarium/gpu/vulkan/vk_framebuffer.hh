/*
 * vk_framebuffer.hh
 * Created by: Evan Kirkiles
 * Created on: Tue Dec 20 2022
 * for Paperarium
 * From: source/blender/gpu/opengl/gl_framebuffer.hh
 */
#ifndef VK_FRAMEBUFFER_HH
#define VK_FRAMEBUFFER_HH

#include "gpu_framebuffer_private.hh"
#include "vk_context.hh"
#include "vk_device.hh"
#include <vulkan/vulkan.h>

namespace paperarium::gpu {

/**
 * @brief Implementation of Framebuffer object using Vulkan
 *
 * A framebuffer defines a set of same-size images that function together
 * for a single frame. Generally, this is a depth/stencil map and several
 * color images, defined as "attachments"––which are essentially just images
 * on the GPU that are managed by the framebuffer.
 *
 * A render pass can only read from/ write to images in the corresponding
 * framebuffer throughout its lifetime.
 *
 * Each framebuffer is associated with a single render pass––framebuffers define
 * all the attachments a render pass will use. It's for this reason that the
 * framebuffer requires a render pass in its initialization struct.
 */
class VKFramebuffer : public Framebuffer {
  friend class VKTexture;

 private:
  /** Context the framebuffer is from.*/
  gpu::VKContext* m_context = nullptr;
  gpu::VKDevice* m_device = nullptr;
  /** Render pass that this frame buffer defines layout for */
  gpu::VKRenderPass* m_render_pass = nullptr;
  /** VkFramebuffer object this class wraps */
  VkFramebuffer m_frame_buffer;
  // dimensions of the frame buffer, in pixels
  uint32_t m_width;
  uint32_t m_height;

  // GLenum gl_attachments_[GPU_FB_MAX_COLOR_ATTACHMENT] = {0};
 public:
  /**
   * Create a conventional frame-buffer to attach texture to.
   */
  VKFramebuffer(char const* name);
  ~VKFramebuffer();

 private:
  void init();
  void update_attachments();
};

/* -------------------------------------------------------------------------- */
/*                               ENUM CONVERSION                              */
/* -------------------------------------------------------------------------- */

/**
 * @brief Converts a GPUAttachmentType to a VkImageUsageFlagBit
 *
 * Used to differentiate between depth stencil attachments and color
 * attachments. To match Blender OpenGL functionality, there are up to 8
 * possible color attachments. Furthermore, Vulkan only supports a combined
 * depth/stencil attachment, instead of separated enum-based depth and stencil
 * attachments. This is (apparently) more in line with modern hardware, as
 * elucidated here:
 * https://stackoverflow.com/questions/16397668/fbo-depth-and-stencil-render-buffer-attachments
 * https://www.reddit.com/r/vulkan/comments/px2igv/comment/hel3qkq/?utm_source=share&utm_medium=web2x&context=3
 *
 * @param type
 * @return VkImageUsageFlagBits
 */
static inline VkImageUsageFlagBits to_vk(const GPUAttachmentType type) {
  switch (type) {
    case GPU_FB_DEPTH_ATTACHMENT:
    case GPU_FB_DEPTH_STENCIL_ATTACHMENT:
      return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
             VK_IMAGE_USAGE_SAMPLED_BIT;
    default:
      return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  }
}

}  // namespace paperarium::gpu

#endif /* VK_FRAMEBUFFER_HH */
