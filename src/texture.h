
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
poof(block_array_h(texture_ptr, {8}, {}))
#include <generated/block_array_h_texture_688856411.h>
