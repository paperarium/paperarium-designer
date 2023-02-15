/*
 * gpu_texture_private.hh
 * Created by: Evan Kirkiles
 * Created on: Tue Dec 20 2022
 * for Paperarium
 * From: source/blender/gpu/intern/gpu_texture_private.hh
 */
#ifndef GPU_TEXTURE_PRIVATE_HH
#define GPU_TEXTURE_PRIVATE_HH

#include "gpu_framebuffer_private.hh"

namespace paperarium::gpu {

typedef enum eGPUTextureFormatFlag {
  GPU_FORMAT_DEPTH = (1 << 0),
  GPU_FORMAT_STENCIL = (1 << 1),
  GPU_FORMAT_INTEGER = (1 << 2),
  GPU_FORMAT_FLOAT = (1 << 3),
  GPU_FORMAT_COMPRESSED = (1 << 4),

  GPU_FORMAT_DEPTH_STENCIL = (GPU_FORMAT_DEPTH | GPU_FORMAT_STENCIL),
} eGPUTextureFormatFlag;

typedef enum eGPUTextureType {
  GPU_TEXTURE_1D = (1 << 0),
  GPU_TEXTURE_2D = (1 << 1),
  GPU_TEXTURE_3D = (1 << 2),
  GPU_TEXTURE_CUBE = (1 << 3),
  GPU_TEXTURE_ARRAY = (1 << 4),
  GPU_TEXTURE_BUFFER = (1 << 5),

  GPU_TEXTURE_1D_ARRAY = (GPU_TEXTURE_1D | GPU_TEXTURE_ARRAY),
  GPU_TEXTURE_2D_ARRAY = (GPU_TEXTURE_2D | GPU_TEXTURE_ARRAY),
  GPU_TEXTURE_CUBE_ARRAY = (GPU_TEXTURE_CUBE | GPU_TEXTURE_ARRAY),
} eGPUTextureType;

/* Maximum number of FBOs a texture can be attached to. */
#define GPU_TEX_MAX_FBO_ATTACHED 32

class Texture {
 public:
  /** Internal Sampler state. */
  eGPUSamplerState sampler_state = GPU_SAMPLER_DEFAULT;
  /** Reference counter. */
  int refcount = 1;
  /** Width & Height (of source data), optional. */
  int src_w = 0, src_h = 0;

 protected:
  /* ---- Texture format (immutable after init). ---- */
  /** Width & Height. For cube-map arrays, d is number of face-layers. */
  int w_, h_, d_;
  /** Internal data format. */
  eGPUTextureFormat format_;
  /** Format characteristics. */
  eGPUTextureFormatFlag format_flag_;
  /** Texture type. */
  eGPUTextureType type_;
  /** Texture usage flags. */
  eGPUTextureUsage gpu_image_usage_flags_;

  /** For debugging */
  char name_[DEBUG_NAME_LEN];

  /** Frame-buffer references to update on deletion. */
  GPUAttachmentType fb_attachment_[GPU_TEX_MAX_FBO_ATTACHED];
  Framebuffer* fb_[GPU_TEX_MAX_FBO_ATTACHED];

 public:
  Texture(char const* name);
  virtual ~Texture();

  // initializes a 2D texture or array of textures (when layers != 0)
  bool init_2d(int w, int h, int layers, eGPUTextureFormat format);
  void usage_set(eGPUTextureUsage usage_flags);

  // texture setting
  void update(eGPUDataFormat format, void const* data);
  virtual void update_sub(eGPUDataFormat format, void const* data) = 0;

  // frame buffer attaching
  void attach_to(Framebuffer* fb, GPUAttachmentType type);
  void detach_from(Framebuffer* fb);

 protected:
  virtual bool init_internal() = 0;
  virtual bool init_internal(GPUTexture const* src, int layer_offset) = 0;
  // virtual bool init_internal(GPUVertBuf* vbo) = 0;
};

/* Syntactic sugar. */
static inline GPUTexture* wrap(Texture* vert) {
  return reinterpret_cast<GPUTexture*>(vert);
}
static inline Texture* unwrap(GPUTexture* vert) {
  return reinterpret_cast<Texture*>(vert);
}
static inline Texture const* unwrap(GPUTexture const* vert) {
  return reinterpret_cast<Texture const*>(vert);
}

inline eGPUTextureFormatFlag to_format_flag(eGPUTextureFormat format) {
  switch (format) {
    case GPU_DEPTH_COMPONENT24:
    case GPU_DEPTH_COMPONENT16:
    case GPU_DEPTH_COMPONENT32F:
      return GPU_FORMAT_DEPTH;
    case GPU_DEPTH24_STENCIL8:
    case GPU_DEPTH32F_STENCIL8:
      return GPU_FORMAT_DEPTH_STENCIL;
    case GPU_R8UI:
    case GPU_RG16I:
    case GPU_R16I:
    case GPU_RG16UI:
    case GPU_R16UI:
    case GPU_R32UI:
      return GPU_FORMAT_INTEGER;
    case GPU_SRGB8_A8_DXT1:
    case GPU_SRGB8_A8_DXT3:
    case GPU_SRGB8_A8_DXT5:
    case GPU_RGBA8_DXT1:
    case GPU_RGBA8_DXT3:
    case GPU_RGBA8_DXT5:
      return GPU_FORMAT_COMPRESSED;
    default:
      return GPU_FORMAT_FLOAT;
  }
}

}  // namespace paperarium::gpu

#endif /* GPU_TEXTURE_PRIVATE_HH */
