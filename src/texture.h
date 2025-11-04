enum texture_storage_format
{
  TextureStorageFormat_Undefined,

  TextureStorageFormat_R16I    = GL_R16I,
  TextureStorageFormat_R16F    = GL_R16F,

  TextureStorageFormat_R32I     = GL_R32I,
  TextureStorageFormat_R32UI    = GL_R32UI,
  TextureStorageFormat_R32F     = GL_R32F,

  TextureStorageFormat_RG16F    = GL_RG16F,
  TextureStorageFormat_RG32F    = GL_RG32F,

  TextureStorageFormat_RGB16F  = GL_RGB16F,
  TextureStorageFormat_RGB32F  = GL_RGB32F,

  TextureStorageFormat_RGBA8   = GL_RGBA8,
  TextureStorageFormat_RGBA16F = GL_RGBA16F,
  TextureStorageFormat_RGBA32F = GL_RGBA32F,

  TextureStorageFormat_Depth32 = GL_DEPTH_COMPONENT32F,
};


#define INVALID_TEXTURE_HANDLE (0xFFFFFFFF)
struct texture
poof(@do_editor_ui)
{
  u32 ID = INVALID_TEXTURE_HANDLE;

  v2i Dim;
  u32 Slices;

  // TODO(Jesse): Channels is implicitly defined in the texture_storage_format.
  // Should we remove this?
  u32 Channels;
  b32 IsDepthTexture;

  texture_storage_format Format;

  cs DebugName;
  b32 Queued;
};


typedef texture* texture_ptr;
poof(block_array_h(texture, {8}, {}))
#include <generated/block_array_h_texture_688856411_0.h>

poof(block_array_h(texture_ptr, {8}, {}))
#include <generated/block_array_h_texture_688856411.h>
