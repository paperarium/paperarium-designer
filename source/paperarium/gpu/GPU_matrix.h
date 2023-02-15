/*
 * GPU_matrix.h
 * Created by: Evan Kirkiles
 * Created on: Mon Dec 19 2022
 * for Paperarium
 * From: source/blender/gpu/GPU_matrix.h
 */
#ifndef GPU_MATRIX_H
#define GPU_MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * To Identity transform & empty stack.
 */
void GPU_matrix_reset(void);

/* ModelView Matrix (2D or 3D) */
void GPU_matrix_push(void); /* TODO: PushCopy vs PushIdentity? */
void GPU_matrix_pop(void);
void GPU_matrix_identity_set(void);

/* 3D ModelView Matrix */
void GPU_matrix_set(float const m[4][4]);
void GPU_matrix_mul(float const m[4][4]);

void GPU_matrix_translate_3f(float x, float y, float z);
void GPU_matrix_translate_3fv(float const vec[3]);
void GPU_matrix_scale_3f(float x, float y, float z);
void GPU_matrix_scale_3fv(float const vec[3]);

#ifdef __cplusplus
}
#endif

#endif /* GPU_MATRIX_H */
