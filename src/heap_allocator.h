// TODO(Jesse): Support alignment
#define HeapAlloc(type, heap, count, ...) \
  (type*)HeapAllocate( (heap), sizeof(type)*count )

enum heap_allocation_type
{
  AllocationType_Free,
  AllocationType_Reserved,

  AllocationType_Error,
};

#define HEAP_MAGIC_NUMBER (0xfecba0123456789)
/* #define HEAP_MAGIC_NUMBER (0xf0f0f0f0f0f0f0f0) */
/* #define HEAP_MAGIC_NUMBER (0xcccccccccccccccc) */
/* #define HEAP_MAGIC_NUMBER (0xFFFFFFFFFFFFFFFF) */
/* #define HEAP_MAGIC_NUMBER (0x0) */

#pragma pack(push, 1)
struct heap_allocation_block
{
  u64 Magic0;
  heap_allocation_type Type;

  // This includes header size and is measured in bytes
  umm Size;

  umm PrevAllocationSize;
  heap_allocation_block *Next;
  umm BaseOffset;
  u64 Magic1;
};
#pragma pack(pop)

poof(are_equal(heap_allocation_block))
#include <generated/are_equal_xuSvDnE8.h>

poof(block_array_h(heap_allocation_block, {256}, {}))
#include <generated/block_array_h_HLVwiHSZ.h>

struct heap_allocator
{
  heap_allocation_block* FirstBlock;
  umm Size;
  bonsai_futex Futex;
};


link_internal b32
IsHeapAllocated(heap_allocator *Heap, void *Allocation);
