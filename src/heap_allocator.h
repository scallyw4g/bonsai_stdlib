enum heap_allocation_type
{
  AllocationType_Free,
  AllocationType_Reserved,
};

#pragma pack(push, 1)
struct heap_allocation_block
{
  heap_allocation_type Type;
  umm Size; // Note(Jesse): Includes sizeof(heap_allocation_block), except for the one on the end of the heap
  umm PrevAllocationSize;
};
#pragma pack(pop)

struct heap_allocator
{
  heap_allocation_block* FirstBlock;
  umm Size;
};

