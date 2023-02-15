/*
 * gpu_texture.cc
 * Created by: Evan Kirkiles
 * Created on: Tue Dec 20 2022
 * for Paperarium
 * From: source/blender/gpu/intern/gpu_texture.cc
 */
#include "GPU_texture.h"
#include "gpu_backend.hh"
#include "gpu_texture_private.hh"
#include <string>

namespace paperarium::gpu {

Texture::Texture(char const* name) {
  if (name) {
    strncpy(name_, name, sizeof(name_));
  } else {
    name_[0] = '\0';
  }
  // ensure all connected framebuffers are not initialized
  for (int i = 0; i < GPU_TEX_MAX_FBO_ATTACHED; i++) {
    fb_[i] = nullptr;
  }
  gpu_image_usage_flags_ = GPU_TEXTURE_USAGE_GENERAL;
}

Texture::~Texture() {
  // remove being attached to all framebuffers
  for (int i = 0; i < GPU_TEX_MAX_FBO_ATTACHED; i++) {
    if (fb_[i] != nullptr) {
      fb_[i]->attachment_remove(fb_attachment_[i]);
    }
  }
}

/* -------------------------------------------------------------------------- */
/*                           TEXTURE INITIALIZATION                           */
/* -------------------------------------------------------------------------- */

/**
 * @brief Initializes a 2D texture on the GPU
 *
 * Textures can then be used as attachments to framebuffers to be read and
 * sampled from in shaders.
 *
 * @param w The width of the texture
 * @param h The height of the texture
 * @param layers The number of layers (if != 0, this is a 2D array)
 * @param format Image format of the texture
 * @return true
 * @return false
 */
bool Texture::init_2d(int w, int h, int layers, eGPUTextureFormat format) {
  w_ = w;
  h_ = h;
  d_ = layers;
  format_ = format;
  format_flag_ = to_format_flag(format);
  type_ = (layers > 0) ? GPU_TEXTURE_2D_ARRAY : GPU_TEXTURE_2D;
  if ((format_flag_ & (GPU_FORMAT_DEPTH_STENCIL | GPU_FORMAT_INTEGER)) == 0) {
    sampler_state = GPU_SAMPLER_FILTER;
  }
  return this->init_internal();
}

/**
 * @brief Sets the GPU image usage flags of the texture
 *
 * @param usage_flags
 */
void Texture::usage_set(eGPUTextureUsage usage_flags) {
  gpu_image_usage_flags_ = usage_flags;
}

/* -------------------------------------------------------------------------- */
/*                             TEXTURE ATTACHMENT                             */
/* -------------------------------------------------------------------------- */

/**
 * @brief Attaches the image to a Framebuffer
 *
 * To be read/written in a shader, textures must be attached to framebuffers.
 *
 * @param fb The framebuffer to attach to
 * @param type The type of attachment this is (e.g. color, depth stencil, etc.)
 */
void Texture::attach_to(Framebuffer* fb, GPUAttachmentType type) {
  for (int i = 0; i < GPU_TEX_MAX_FBO_ATTACHED; i++) {
    if (fb_[i] == nullptr) {
      fb_attachment_[i] = type;
      fb_[i] = fb;
      return;
    }
  }
  printf("GPU: Error: Texture: Not enough attachment");
}

/**
 * @brief Detaches the image from a Framebuffer
 *
 * To be read/written in a shader, textures must be attached to framebuffers.
 * This removes a texture from a framebuffer, i.e. when it is no longer needed.
 *
 * @param fb The framebuffer to detach from
 */
void Texture::detach_from(Framebuffer* fb) {
  for (int i = 0; i < GPU_TEX_MAX_FBO_ATTACHED; i++) {
    if (fb_[i] == fb) {
      fb_[i]->attachment_remove(fb_attachment_[i]);
      fb_[i] = nullptr;
      return;
    }
  }
  printf("GPU: Error: Texture: Framebuffer is not attached");
}

/* -------------------------------------------------------------------------- */
/*                               TEXTURE SETTING                              */
/* -------------------------------------------------------------------------- */

void Texture::update(eGPUDataFormat format, void const* data) {
  this->update_sub(format, data);
}

}  // namespace paperarium::gpu

using namespace paperarium;
using namespace paperarium::gpu;

/**
 * @brief Creates a texture on the GPU
 *
 * Textures can then be used as attachments to framebuffers to be read and
 * sampled from in shaders.
 *
 * @param name A debug name of the texture
 * @param w The width of the texture, in pixels
 * @param h The height of the texture, in pixels
 * @param d The depth / number of layers of the texture
 * @param type The type of texture
 * @param tex_format The format the texture should be stored in
 * @param data_format
 * @param usage How this texture will be used (e.g. color, depth)
 * @param pixels Data to initialize the texture with
 * @return GPUTexture*
 */
static inline GPUTexture* gpu_texture_create(
    char const* name, int const w, int const h, int const d,
    const eGPUTextureType type, eGPUTextureFormat tex_format,
    eGPUDataFormat data_format, eGPUTextureUsage usage, void const* pixels) {
  // use GPU-specific texture allocation function
  Texture* tex = GPUBackend::get()->texture_alloc(name);
  bool success = false;
  switch (type) {
    case GPU_TEXTURE_2D:
    case GPU_TEXTURE_2D_ARRAY:
      success = tex->init_2d(w, h, d, tex_format);
      break;
    default:
      printf("ERROR: GPU: Texture: Only 2D textures supported for now!");
      break;
  }
  // assign usage
  tex->usage_set(usage);
  // check success
  if (!success) {
    delete tex;
    return nullptr;
  }
  if (pixels) {
    tex->update(data_format, pixels);
  }
  return reinterpret_cast<GPUTexture*>(tex);
}