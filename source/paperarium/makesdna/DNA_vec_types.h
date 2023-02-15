/*
 * DNA_vec_types.h
 * Created by: Evan Kirkiles
 * Created on: Mon Dec 19 2022
 * for Paperarium
 * From: source/blender/makesdna/DNA_vec_types.h
 */

#ifndef DNA_VEC_TYPES_H
#define DNA_VEC_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/** integer rectangle. */
typedef struct rcti {
  int xmin, xmax;
  int ymin, ymax;
} rcti;

#ifdef __cplusplus
}
#endif

#endif /* DNA_VEC_TYPES_H */
