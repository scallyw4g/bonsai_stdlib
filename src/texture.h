
struct texture
{
  u32 ID;
  v2i Dim;

  u32 Slices;
};

typedef texture* texture_ptr;
poof(block_array_h(texture, {8}))
#include <generated/block_array_h_texture_688856411.h>
