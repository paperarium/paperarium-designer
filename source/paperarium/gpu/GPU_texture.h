/*
 * GPU_texture.h
 * Created by: Evan Kirkiles
 * Created on: Mon Dec 19 2022
 * for Paperarium
 * From: source/blender/gpu/GPU_texture.h
 */

#ifndef GPU_TEXTURE_H
#define GPU_TEXTURE_H

/** Opaque type hiding blender::gpu::Texture. */
typedef struct GPUTexture GPUTexture;

/**
 * GPU Samplers state
 * - Specify the sampler state to bind a texture with.
 * - Internally used by textures.
 * - All states are created at startup to avoid runtime costs.
 */
typedef enum eGPUSamplerState {
  GPU_SAMPLER_DEFAULT = 0,
  GPU_SAMPLER_FILTER = (1 << 0),
  GPU_SAMPLER_MIPMAP = (1 << 1),
  GPU_SAMPLER_REPEAT_S = (1 << 2),
  GPU_SAMPLER_REPEAT_T = (1 << 3),
  GPU_SAMPLER_REPEAT_R = (1 << 4),
  GPU_SAMPLER_CLAMP_BORDER =
      (1 << 5), /* Clamp to border color instead of border texel. */
  GPU_SAMPLER_COMPARE = (1 << 6),
  GPU_SAMPLER_ANISO = (1 << 7),
  GPU_SAMPLER_MIRROR_REPEAT =
      (1 << 8), /* Requires any REPEAT flag to be set. */
  GPU_SAMPLER_ICON = (1 << 9),

  GPU_SAMPLER_REPEAT =
      (GPU_SAMPLER_REPEAT_S | GPU_SAMPLER_REPEAT_T | GPU_SAMPLER_REPEAT_R),
} eGPUSamplerState;

/* GPU Texture
 * - always returns unsigned char RGBA textures
 * - if texture with non square dimensions is created, depending on the
 *   graphics card capabilities the texture may actually be stored in a
 *   larger texture with power of two dimensions.
 * - can use reference counting:
 *   - reference counter after GPU_texture_create is 1
 *   - GPU_texture_ref increases by one
 *   - GPU_texture_free decreases by one, and frees if 0
 * - if created with from_blender, will not free the texture
 */

/**
 * Wrapper to supported OpenGL/Vulkan texture internal storage
 * If you need a type just un-comment it. Be aware that some formats
 * are not supported by render-buffers. All of the following formats
 * are part of the OpenGL 3.3 core
 * specification.
 */
typedef enum eGPUTextureFormat {
  /* Formats texture & render-buffer. */
  GPU_RGBA8UI,
  GPU_RGBA8I,
  GPU_RGBA8,
  GPU_RGBA32UI,
  GPU_RGBA32I,
  GPU_RGBA32F,
  GPU_RGBA16UI,
  GPU_RGBA16I,
  GPU_RGBA16F,
  GPU_RGBA16,
  GPU_RG8UI,
  GPU_RG8I,
  GPU_RG8,
  GPU_RG32UI,
  GPU_RG32I,
  GPU_RG32F,
  GPU_RG16UI,
  GPU_RG16I,
  GPU_RG16F,
  GPU_RG16,
  GPU_R8UI,
  GPU_R8I,
  GPU_R8,
  GPU_R32UI,
  GPU_R32I,
  GPU_R32F,
  GPU_R16UI,
  GPU_R16I,
  GPU_R16F,
  GPU_R16, /* Max texture buffer format. */

  /* Special formats texture & render-buffer. */
  GPU_RGB10_A2,
  GPU_R11F_G11F_B10F,
  GPU_DEPTH32F_STENCIL8,
  GPU_DEPTH24_STENCIL8,
  GPU_SRGB8_A8,
#if 0
  GPU_RGB10_A2UI,
#endif

  /* Texture only format */
  GPU_RGB16F,
#if 0
  GPU_RGBA16_SNORM,
  GPU_RGBA8_SNORM,
  GPU_RGB32F,
  GPU_RGB32I,
  GPU_RGB32UI,
  GPU_RGB16_SNORM,
  GPU_RGB16I,
  GPU_RGB16UI,
  GPU_RGB16,
  GPU_RGB8_SNORM,
  GPU_RGB8,
  GPU_RGB8I,
  GPU_RGB8UI,
  GPU_RG16_SNORM,
  GPU_RG8_SNORM,
  GPU_R16_SNORM,
  GPU_R8_SNORM,
#endif

  /* Special formats texture only */
  GPU_SRGB8_A8_DXT1,
  GPU_SRGB8_A8_DXT3,
  GPU_SRGB8_A8_DXT5,
  GPU_RGBA8_DXT1,
  GPU_RGBA8_DXT3,
  GPU_RGBA8_DXT5,
#if 0
  GPU_SRGB8,
  GPU_RGB9_E5,
  GPU_COMPRESSED_RG_RGTC2,
  GPU_COMPRESSED_SIGNED_RG_RGTC2,
  GPU_COMPRESSED_RED_RGTC1,
  GPU_COMPRESSED_SIGNED_RED_RGTC1,
#endif

  /* Depth Formats */
  GPU_DEPTH_COMPONENT32F,
  GPU_DEPTH_COMPONENT24,
  GPU_DEPTH_COMPONENT16,
} eGPUTextureFormat;

typedef enum eGPUDataFormat {
  GPU_DATA_FLOAT,
  GPU_DATA_INT,
  GPU_DATA_UINT,
  GPU_DATA_UBYTE,
  GPU_DATA_UINT_24_8,
  GPU_DATA_10_11_11_REV,
  GPU_DATA_2_10_10_10_REV,
  GPU_DATA_HALF_FLOAT
} eGPUDataFormat;

/** Texture usage flags.
 * Texture usage flags allow backend implementations to contextually optimize
 * texture resources. Any texture with an explicit flag should not perform
 * operations which are not explicitly specified in the usage flags. If usage is
 * unknown upfront, then GPU_TEXTURE_USAGE_GENERAL can be used.
 *
 * NOTE: These usage flags act as hints for the backend implementations. There
 * may be no benefit in some circumstances, and certain resource types may
 * insert additional usage as required. However, explicit usage can ensure that
 * hardware features such as render target/texture compression can be used. For
 * explicit APIs such as Metal/Vulkan, texture usage needs to be specified
 * up-front.
 */
typedef enum eGPUTextureUsage {
  /* Whether texture is sampled or read during a shader. */
  GPU_TEXTURE_USAGE_SHADER_READ = (1 << 0),
  /* Whether the texture is written to by a shader using imageStore. */
  GPU_TEXTURE_USAGE_SHADER_WRITE = (1 << 1),
  /* Whether a texture is used as an attachment in a frame-buffer. */
  GPU_TEXTURE_USAGE_ATTACHMENT = (1 << 2),
  /* Whether the texture is used as a texture view, uses mip-map layer
   * adjustment, OR, uses swizzle access masks. Mip-map base layer adjustment
   * and texture channel swizzling requires a texture view under-the-hood. */
  GPU_TEXTURE_USAGE_MIP_SWIZZLE_VIEW = (1 << 3),
  /* Whether a texture can be allocated without any backing memory. It is used
   * as an attachment to store data, but is not needed by any future passes.
   * This usage mode should be used in scenarios where an attachment has no
   * previous contents and is not stored after a render pass. */
  GPU_TEXTURE_USAGE_MEMORYLESS = (1 << 4),
  /* Whether the texture needs to be read from by the CPU. */
  GPU_TEXTURE_USAGE_HOST_READ = (1 << 5),
  GPU_TEXTURE_USAGE_GENERAL = 0xFF,
} eGPUTextureUsage;

#endif /* GPU_TEXTURE_H */
