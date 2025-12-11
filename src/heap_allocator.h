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
  umm Magic0;
  heap_allocation_type Type;
  umm Size; // Note(Jesse): Includes sizeof(heap_allocation_block), except for the one on the end of the heap
  umm PrevAllocationSize;
  umm Magic1;
};
#pragma pack(pop)

struct heap_allocator
{
  heap_allocation_block* FirstBlock;
  umm Size;
  bonsai_futex Futex;
};


link_internal b32
IsHeapAllocated(heap_allocator *Heap, void *Allocation);
