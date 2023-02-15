/*
 * gpu_context.cc
 * Created by: Evan Kirkiles
 * Created on: Mon Dec 19 2022
 * for Paperarium
 * From: source/blender/gpu/intern/gpu_context.cc
 */

#include "GPU_context.h"
#include "gpu_backend.hh"
#include "gpu_common.h"
#include "gpu_context_private.hh"
#include "gpu_matrix_private.hh"
#include "vk_backend.hh"
#include <mutex>

using namespace paperarium::gpu;

/* -------------------------------------------------------------------------- */
/*                       GPUContext Abstract Base Class                       */
/* -------------------------------------------------------------------------- */
// All GPUContext classes inherit from this Context which defines how GPU
// contexts behave.

// keep track of active context and users
static thread_local Context* active_ctx = nullptr;
static std::mutex backend_users_mutex;
static int num_backend_users = 0;
static void gpu_backend_create();
static void gpu_backend_discard();

namespace paperarium::gpu {

int Context::context_counter = 0;
Context::Context() {
  thread_ = pthread_self();
  is_active_ = false;
  matrix_state = GPU_matrix_state_create();
}

Context::~Context() {
  GPU_matrix_state_discard(matrix_state);
  // delete state_manager;
  // delete front_left;
  // delete back_left;
}

bool Context::is_active_on_thread() {
  return (this == active_ctx) && pthread_equal(pthread_self(), thread_);
}

Context* Context::get() { return active_ctx; }

}  // namespace paperarium::gpu

/* -------------------------------------------------------------------------- */
/*                                 C INTERFACE                                */
/* -------------------------------------------------------------------------- */
// implementation for GPU_context code. these create and discard new GPU
// contexts, which represent abstractions of GPU functionality (GL, VK, etc.)

/**
 * @brief Creates a GPU context
 *
 * First run creates context, second one allocates a new backend.
 *
 * @param ghost_window
 * @param ghost_context
 * @return GPUContext*
 */
GPUContext* GPU_context_create(PLATF_SURF_PARAMS) {
  {
    std::scoped_lock lock(backend_users_mutex);
    if (num_backend_users == 0) {
      /* Automatically create backend when first context is created. */
      gpu_backend_create();
    }
    num_backend_users++;
  }
  Context* ctx = GPUBackend::get()->context_alloc(PLATF_SURF_ARGS);
  GPU_context_active_set(wrap(ctx));
  return wrap(ctx);
}

/**
 * @brief Destroys the GPU context
 *
 * @param ctx_
 */
void GPU_context_discard(GPUContext* ctx_) {
  Context* ctx = unwrap(ctx_);
  delete ctx;
  active_ctx = nullptr;
  {
    std::scoped_lock lock(backend_users_mutex);
    num_backend_users--;
    assert(num_backend_users >= 0);
    if (num_backend_users == 0) {
      /* Discard backend when last context is discarded. */
      gpu_backend_discard();
    }
  }
}

/* ---------------------------- CONTEXT STATEFULS --------------------------- */

/**
 * @brief Sets the active GPU context for rendering
 *
 * @param ctx_ A GPU context to use for rendering
 */
void GPU_context_active_set(GPUContext* ctx_) {
  Context* ctx = unwrap(ctx_);
  if (active_ctx) active_ctx->deactivate();
  active_ctx = ctx;
  if (ctx) ctx->activate();
}

/**
 * @brief Gets the currently active GPU context
 *
 * @return GPUContext*
 */
GPUContext* GPU_context_active_get() { return wrap(Context::get()); }

/* -------------------------------- TIMELINE -------------------------------- */

void GPU_context_begin_frame(GPUContext* ctx) {
  paperarium::gpu::Context* _ctx = unwrap(ctx);
  if (_ctx) {
    _ctx->begin_frame();
  }
}

void GPU_context_end_frame(GPUContext* ctx) {
  paperarium::gpu::Context* _ctx = unwrap(ctx);
  if (_ctx) {
    _ctx->end_frame();
  }
}

/**
 * @brief Main context global mutex
 * Used to avoid crash on some old drivers.
 */
static std::mutex main_context_mutex;
void GPU_context_main_lock() { main_context_mutex.lock(); }
void GPU_context_main_unlock() { main_context_mutex.unlock(); }

/* -------------------------------------------------------------------------- */
/*                                 GPUBackend                                 */
/* -------------------------------------------------------------------------- */
// whereas the context defines the basic functionality of the GPU API, the
// backend wraps the primitives of the GPU API so that they all function the
// same.

static eGPUBackendType g_backend_type = GPU_BACKEND_VULKAN;
static GPUBackend* g_backend = nullptr;

/**
 * @brief Selects a GPU backend type
 *
 * @param backend
 */
void GPU_backend_type_selection_set(const eGPUBackendType backend) {
  g_backend_type = backend;
}

/**
 * @brief Returns the currently selected type of backend
 *
 * @return eGPUBackendType
 */
eGPUBackendType GPU_backend_type_selection_get() { return g_backend_type; }

static void gpu_backend_create() {
  assert(g_backend == nullptr);
  assert(GPU_backend_supported());

  switch (g_backend_type) {
    case GPU_BACKEND_VULKAN:
      g_backend = new VKBackend;
      break;
    default:
      assert(0);
      break;
  }
}

void gpu_backend_delete_resources() {
  assert(g_backend);
  g_backend->delete_resources();
}

void gpu_backend_discard() {
  /* TODO: assert no resource left. */
  delete g_backend;
  g_backend = nullptr;
}

eGPUBackendType GPU_backend_get_type() {
  if (g_backend && dynamic_cast<VKBackend*>(g_backend) != nullptr) {
    return GPU_BACKEND_VULKAN;
  }
  return GPU_BACKEND_NONE;
}

// Returns the backend for the context allocated in this file
GPUBackend* GPUBackend::get() { return g_backend; }