enum texture_storage_format
{
  TextureStorageFormat_Undefined,

  TextureStorageFormat_R16F    = GL_R16F,
  TextureStorageFormat_R32F    = GL_R32F,

  TextureStorageFormat_RGB16F  = GL_RGB16F,
  TextureStorageFormat_RGB32F  = GL_RGB32F,

  TextureStorageFormat_RGBA8   = GL_RGBA8,
  TextureStorageFormat_RGBA16F = GL_RGBA16F,
  TextureStorageFormat_RGBA32F = GL_RGBA32F,
};


struct texture
{
  u32 ID;

  v2i Dim;
  u32 Slices;

  u32 Channels;
  b32 IsDepthTexture;

  cs DebugName;
};


typedef texture* texture_ptr;
poof(block_array_h(texture, {8}, {}))
#include <generated/block_array_h_texture_688856411_0.h>

poof(block_array_h(texture_ptr, {8}, {}))
#include <generated/block_array_h_texture_688856411.h>
