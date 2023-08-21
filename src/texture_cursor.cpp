poof(are_equal(texture))
#include <generated/are_equal_texture.h>

link_internal b32
AreEqual(texture Thing1, texture Thing2)
{
  b32 Result = MemoryIsEqual((u8*)&Thing1, (u8*)&Thing2, sizeof( texture ) );
  return Result;
}

link_internal void
DeepCopy(texture *E0, texture *E1)
{
  *E1 = *E0;
}

poof(generate_cursor(texture))
#include <generated/generate_cursor_texture.h>
