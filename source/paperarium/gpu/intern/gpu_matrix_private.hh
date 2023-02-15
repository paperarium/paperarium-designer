/*
 * gpu_matrix_private.hh
 * Created by: Evan Kirkiles
 * Created on: Mon Dec 19 2022
 * for Paperarium
 * From: source/blender/gpu/intern/gpu_matrix_private.hh
 */
#ifndef GPU_MATRIX_PRIVATE_HH
#define GPU_MATRIX_PRIVATE_HH

// GLM
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct GPUMatrixState* GPU_matrix_state_create(void);
void GPU_matrix_state_discard(struct GPUMatrixState* state);

#endif /* GPU_MATRIX_PRIVATE_HH */
