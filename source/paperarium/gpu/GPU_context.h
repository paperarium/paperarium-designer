/*
 * GPU_context.h
 * Created by: Evan Kirkiles
 * Created on: Mon Dec 19 2022
 * for Paperarium
 * From: source/blender/gpu/GPU_context.h
 */

#ifndef GPU_CONTEXT_H
#define GPU_CONTEXT_H

#include "GPU_platform.h"
#include "gpu_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* GPU back-ends abstract the differences between different APIs.
 * #GPU_context_create automatically initializes the back-end, and
 * #GPU_context_discard frees it when there are no more contexts. */
bool GPU_backend_supported(void);
void GPU_backend_type_selection_set(const eGPUBackendType backend);
eGPUBackendType GPU_backend_type_selection_get(void);
eGPUBackendType GPU_backend_get_type(void);

/** Opaque type hiding blender::gpu::Context. */
typedef struct GPUContext GPUContext;

GPUContext* GPU_context_create(PLATF_SURF_PARAMS);
/**
 * To be called after #GPU_context_active_set(ctx_to_destroy).
 */
void GPU_context_discard(GPUContext*);

/**
 * Ctx can be NULL.
 */
void GPU_context_active_set(GPUContext*);
GPUContext* GPU_context_active_get(void);

/* Legacy GPU (Intel HD4000 series) do not support sharing GPU objects between
 * GPU contexts. EEVEE/Workbench can create different contexts for image/preview
 * rendering, baking or compiling. When a legacy GPU is detected
 * (`GPU_use_main_context_workaround()`) any worker threads should use the draw
 * manager opengl context and make sure that they are the only one using it by
 * locking the main context using these two functions. */
void GPU_context_main_lock(void);
void GPU_context_main_unlock(void);

/* GPU Begin/end work blocks */
void GPU_render_begin(void);
void GPU_render_end(void);

/* For operations which need to run exactly once per frame -- even if there are
 * no render updates.
 */
void GPU_render_step(void);

#ifdef __cplusplus
}
#endif

#endif /* GPU_CONTEXT_H */
