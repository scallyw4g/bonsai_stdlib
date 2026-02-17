u32
ChrisWellonsIntegerHash_lowbias32(u32 x)
{
  u32 Result = x;
  Result = Result ^ (Result >> 16);
  Result = Result ^ 0x7feb352d;
  Result = Result ^ (Result >> 15);
  Result = Result * 0x846ca68b;
  Result = Result ^ (Result >> 16);
  return Result;
}


poof(block_array_h(u32, {8}, {}))
#include <generated/block_array_h_u32_688856411.h>


struct u32_hashtable_wrapper
{
  u32 N;
};

link_inline umm Hash(u32 *E) { return umm(ChrisWellonsIntegerHash_lowbias32(*E)); }
link_inline umm Hash(u64 *E) { NotImplemented; return 0; }

link_inline umm
Hash(u32_hashtable_wrapper *E) { return Hash(&E->N); }

poof(maybe(u32))
#include <generated/maybe_u32.h>
/* poof(are_equal(u32)) */
/* #include <generated/are_equal_u32_hashtable_wrapper.h> */

poof(hashtable_struct(u32))
#include <generated/hashtable_struct_u32.h>
