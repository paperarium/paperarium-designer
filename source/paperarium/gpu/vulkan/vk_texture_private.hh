/*
 * vk_texture_private.hh
 * Created by: Evan Kirkiles
 * Created on: Tue Dec 20 2022
 * for Paperarium
 * From: source/blender/gpu/opengl/gl_texture.hh
 */
#ifndef VK_TEXTURE_PRIVATE_HH
#define VK_TEXTURE_PRIVATE_HH

#include "gpu_texture_private.hh"
#include "vk_framebuffer.hh"
#include <vulkan/vulkan.h>
#include <assert.h>

namespace paperarium::gpu {

class VKTexture : public Texture {
  friend class VKFramebuffer;

 private:
  /** The image view to make the image accessible in shaders */
  VkImageView view = VK_NULL_HANDLE;
  /** The image object on the GPU */
  VkImage image = VK_NULL_HANDLE;
  /** The device memory in which the image is stored */
  VkDeviceMemory memory = VK_NULL_HANDLE;
};

/** From:
 * https://github.com/KhronosGroup/KTX-Software/blob/1768624f33f8767aa4ad73d6e3a218847d92e489/lib/vk_format.h#L677-L678
 */
inline VkFormat to_vk_internal_format(eGPUTextureFormat format) {
  switch (format) {
    case GPU_RGBA8UI:
      return VK_FORMAT_R8G8B8A8_UINT;
    case GPU_RGBA8I:
      return VK_FORMAT_R8G8B8A8_SINT;
    case GPU_RGBA8:
      return VK_FORMAT_R8G8B8A8_UNORM;
    case GPU_RGBA32UI:
      return VK_FORMAT_R32G32B32A32_UINT;
    case GPU_RGBA32I:
      return VK_FORMAT_R32G32B32A32_SINT;
    case GPU_RGBA32F:
      return VK_FORMAT_R32G32B32A32_SFLOAT;
    case GPU_RGBA16UI:
      return VK_FORMAT_R16G16B16A16_UINT;
    case GPU_RGBA16I:
      return VK_FORMAT_R16G16B16A16_SINT;
    case GPU_RGBA16F:
      return VK_FORMAT_R16G16B16A16_SFLOAT;
    case GPU_RGBA16:
      return VK_FORMAT_R16G16B16A16_UNORM;
    case GPU_RG8UI:
      return VK_FORMAT_R8G8_UINT;
    case GPU_RG8I:
      return VK_FORMAT_R8G8_SINT;
    case GPU_RG8:
      return VK_FORMAT_R8G8_UNORM;
    case GPU_RG32UI:
      return VK_FORMAT_R32G32_UINT;
    case GPU_RG32I:
      return VK_FORMAT_R32G32_SINT;
    case GPU_RG32F:
      return VK_FORMAT_R32G32_SFLOAT;
    case GPU_RG16UI:
      return VK_FORMAT_R16G16_UINT;
    case GPU_RG16I:
      return VK_FORMAT_R16G16_SINT;
    case GPU_RG16F:
      return VK_FORMAT_R16G16_SFLOAT;
    case GPU_RG16:
      return VK_FORMAT_R16G16_SINT;
    case GPU_R8UI:
      return VK_FORMAT_R8_UINT;
    case GPU_R8I:
      return VK_FORMAT_R8_SINT;
    case GPU_R8:
      return VK_FORMAT_R8_UNORM;
    case GPU_R32UI:
      return VK_FORMAT_R32_UINT;
    case GPU_R32I:
      return VK_FORMAT_R32_SINT;
    case GPU_R32F:
      return VK_FORMAT_R32_SFLOAT;
    case GPU_R16UI:
      return VK_FORMAT_R16_UINT;
    case GPU_R16I:
      return VK_FORMAT_R16_SINT;
    case GPU_R16F:
      return VK_FORMAT_R16_SFLOAT;
    case GPU_R16:
      return VK_FORMAT_R16_UNORM;
    /* Special formats texture & renderbuffer */
    case GPU_RGB10_A2:
      return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
    case GPU_R11F_G11F_B10F:
      return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    case GPU_DEPTH32F_STENCIL8:
      return VK_FORMAT_D32_SFLOAT_S8_UINT;
    case GPU_DEPTH24_STENCIL8:
      return VK_FORMAT_D24_UNORM_S8_UINT;
    case GPU_SRGB8_A8:
      return VK_FORMAT_R8G8B8A8_SRGB;
    /* Texture only format */
    case GPU_RGB16F:
      return VK_FORMAT_R16G16B16_SFLOAT;
    /* Special formats texture only */
    case GPU_SRGB8_A8_DXT1:
      return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    case GPU_SRGB8_A8_DXT3:
      return VK_FORMAT_BC2_SRGB_BLOCK;
    case GPU_SRGB8_A8_DXT5:
      return VK_FORMAT_BC3_SRGB_BLOCK;
    case GPU_RGBA8_DXT1:
      return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case GPU_RGBA8_DXT3:
      return VK_FORMAT_BC3_UNORM_BLOCK;
    case GPU_RGBA8_DXT5:
      return VK_FORMAT_BC2_UNORM_BLOCK;
    /* Depth Formats */
    case GPU_DEPTH_COMPONENT32F:
      return VK_FORMAT_D32_SFLOAT;
    case GPU_DEPTH_COMPONENT24:
      return VK_FORMAT_X8_D24_UNORM_PACK32;
    case GPU_DEPTH_COMPONENT16:
      return VK_FORMAT_D16_UNORM;
    default:
      assert(0);
      printf("Texture format incorrect or unsupported");
      return VK_FORMAT_UNDEFINED;
  }
}

inline VkImageViewType to_vk(eGPUTextureType type) {
  switch (type) {
    case GPU_TEXTURE_1D:
      return VK_IMAGE_VIEW_TYPE_1D;
    case GPU_TEXTURE_1D_ARRAY:
      return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
    case GPU_TEXTURE_2D:
      return VK_IMAGE_VIEW_TYPE_2D;
    case GPU_TEXTURE_2D_ARRAY:
      return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    case GPU_TEXTURE_3D:
      return VK_IMAGE_VIEW_TYPE_3D;
    case GPU_TEXTURE_CUBE:
      return VK_IMAGE_VIEW_TYPE_CUBE;
    case GPU_TEXTURE_CUBE_ARRAY:
      return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
    default:
      assert(0);  // fail if invalid texture type
      return VK_IMAGE_VIEW_TYPE_1D;
  }
}

}  // namespace paperarium::gpu

#endif /* VK_TEXTURE_PRIVATE_HH */
