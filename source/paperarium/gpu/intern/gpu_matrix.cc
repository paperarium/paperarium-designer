/*
 * gpu_matrix.cc
 * Created by: Evan Kirkiles
 * Created on: Mon Dec 19 2022
 * for Paperarium
 * From: source/blender/gpu/intern/gpu_matrix.cc
 */

#include "gpu_matrix_private.hh"

namespace paperarium::gpu {

#define MATRIX_STACK_DEPTH 32

struct MatrixStack {
  glm::mat4 stack[MATRIX_STACK_DEPTH];
  uint16_t top;
};
struct GPUMatrixState {
  MatrixStack model_view_stack;
  MatrixStack projection_stack;
  bool dirty;
};

#define ModelViewStack Context::get()->matrix_state->model_view_stack
#define ModelView ModelViewStack.stack[ModelViewStack.top]

/**
 * @brief Allocates an Identity 4x4 matrix
 *
 * @return GPUMatrixState*
 */
GPUMatrixState* GPU_matrix_state_create() {
  GPUMatrixState* state = (GPUMatrixState*)malloc(sizeof(*state));
  const MatrixStack identity_stack = {{glm::mat4(1.0f)}, 0};
  state->model_view_stack = state->projection_stack = identity_stack;
  state->dirty = true;
  return state;
}

void GPU_matrix_state_discard(GPUMatrixState* state) { free(state); }

}  // namespace paperarium::gpu