/*
 * DNA_view3d_types.h
 * Created by: Evan Kirkiles
 * Created on: Fri Dec 16 2022
 * for Paperarium
 */
#ifndef DNA_VIEW3D_TYPES_H
#define DNA_VIEW3D_TYPES_H

// SEE: DNA_view3d_types.h

typedef struct RegionView3D {
  /** Projection matrix */
  float winmat[4][4];
  /** Model view matrix */
  float viewmat[4][4];
  /** Inverse of viewmat. */
  float viewinv[4][4];
  /** Viewmat*winmat. */
  float persmat[4][4];
  /** Inverse of persmat. */
  float persinv[4][4];

  /** viewmat/persmat multiplied with object matrix, while drawing and
   * selection. */
  float viewmatob[4][4];
  float persmatob[4][4];

  /** View rotation, must be kept normalized. */
  float viewquat[4];

  /**
   * View center & orbit pivot, negative of world-space location,
   * also matches `-viewinv[3][0:3]` in orthographic mode.
   */
  float ofs[3];
  /** Viewport zoom on the camera frame */
  float camzoom;

  /** Persp / ortho view */
  char persp;
} RegionView3D;

typedef struct View3DShading {
  /** Shading type (OB_SOLID, ..). */
  char type;
  /** Runtime, for toggle between rendered viewport. */
  char prev_type;
  char prev_type_wire;
} View3DShading;

typedef struct View3D_Runtime {
  /** Nkey panel stores stuff here. */
  void* properties_storage;
  /** Runtime only flags. */
  int flag;

  char _pad1[4];
  /* Only used for overlay stats while in local-view. */
  // struct SceneStats* local_stats;
} View3D_Runtime;

typedef struct View3D {
  /** Display settings. */
  View3DShading shading;
  View3DOverlay overlay;

  /** Runtime evaluation data (keep last). */
  View3D_Runtime runtime;
};

#endif /* DNA_VIEW3D_TYPES_H */
