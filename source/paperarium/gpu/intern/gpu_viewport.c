/*
 * gpu_viewport.cc
 * Created by: Evan Kirkiles
 * Created on: Mon Dec 19 2022
 * for Paperarium
 * From: source/blender/gpu/intern/gpu_viewport.c
 */

#include "GPU_viewport.h"

/* Struct storing a viewport specific GPUBatch.
 * The end-goal is to have a single batch shared across viewport and use a model
 * matrix to place the batch. Due to OCIO and Image/UV editor we are not able to
 * use an model matrix yet. */
struct GPUViewportBatch {
  GPUBatch* batch;
  struct {
    rctf rect_pos;
    rctf rect_uv;
  } last_used_parameters;
};

static struct {
  GPUVertFormat format;
  struct {
    uint pos, tex_coord;
  } attr_id;
} g_viewport = {{0}};

struct GPUViewport {
  int size[2];
  int flag;

  /* Set the active view (for stereoscopic viewport rendering). */
  int active_view;

  /* Viewport Resources. */
  struct DRWData* draw_data;
  /** Color buffers. */
  GPUTexture* color_render_tx;
  GPUTexture* color_overlay_tx;
  /** Depth buffer. Can be shared with GPUOffscreen. */
  GPUTexture* depth_tx;
  /** Overlay framebuffer for drawing outside of DRW module. */
  GPUFrameBuffer* overlay_fb;

  // the batch
  struct GPUViewportBatch batch;
};