/*
 * vk_texture.cc
 * Created by: Evan Kirkiles
 * Created on: Tue Dec 20 2022
 * for Paperarium
 * From: source/blender/gpu/opengl/gl_texture.cc
 */
#include "vk_texture_private.hh"

namespace paperarium::gpu {

VKTexture::VKTexture() { assert(VKContext::get() != nullptr); }

}  // namespace paperarium::gpu