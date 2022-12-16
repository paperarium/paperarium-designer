/*
 * context.c
 * Created by: Evan Kirkiles
 * Created on: Fri Dec 16 2022
 * for Paperarium
 */

#include "PKE_context.h"

struct pContext {
  struct {
    void* scene;
  } data;
}