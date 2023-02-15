/*
 * GPU_viewport.h
 * Created by: Evan Kirkiles
 * Created on: Mon Dec 19 2022
 * for Paperarium
 */
#ifndef GPU_VIEWPORT_H
#define GPU_VIEWPORT_H

#include "DNA_vec_types.h"
#include "GPU_texture.h"

typedef struct GPUViewport GPUViewport;

GPUViewport* GPU_viewport_create(void);
GPUViewport* GPU_viewport_stereo_create(void);
void GPU_viewport_bind(GPUViewport* viewport, int view, rcti const* rect);
void GPU_viewport_unbind(GPUViewport* viewport);

#endif /* GPU_VIEWPORT_H */
