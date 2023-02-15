/*
 * GPU_framebuffer.h
 * Created by: Evan Kirkiles
 * Created on: Tue Dec 20 2022
 * for Paperarium
 * From: source/blender/gpu/GPU_framebuffer.h
 */
#ifndef GPU_FRAMEBUFFER_H
#define GPU_FRAMEBUFFER_H

#include "GPU_texture.h"

typedef enum eGPUFramebufferBits {
  GPU_COLOR_BIT = (1 << 0),
  GPU_DEPTH_BIT = (1 << 1),
  GPU_STENCIL_BIT = (1 << 2),
} eGPUFramebufferBits;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GPUAttachment {
  struct GPUTexture* tex;
  int layer, mip;
} GPUAttachment;

/** Opaque type hiding blender::gpu::Framebuffer. */
typedef struct GPUFramebuffer GPUFramebuffer;

GPUFramebuffer* GPU_framebuffer_create(char const* name);
void GPU_framebuffer_free(GPUFramebuffer* fb);
void GPU_framebuffer_bind(GPUFramebuffer* fb);

char const* GPU_framebuffer_get_name(GPUFramebuffer* fb);

// quick-initializers for GPU attachment types
#define GPU_ATTACHMENT_NONE \
  { NULL, -1, 0, }

#ifdef __cplusplus
}
#endif

#endif /* GPU_FRAMEBUFFER_H */
