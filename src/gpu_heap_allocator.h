#pragma once

struct gpu_heap_allocation
{
  heap_allocation_type Type;

  /* untextured_3d_geometry_buffer Data; */
  umm BaseOffset;
  umm BaseOffsetInElements;
  umm SizeInElements;
  gpu_heap_allocation *Prev;
  gpu_heap_allocation *Next;
};

struct gpu_heap_allocator
{
  bonsai_futex Futex;

  gpu_mapped_element_buffer Storage;

  // Linked list of all allocations
  gpu_heap_allocation *FirstBlock;

  memory_arena *gpu_heap_allocation_Memory;
  gpu_heap_allocation *gpu_heap_allocation_Freelist;
};


link_internal gpu_heap_allocator
InitGpuHeap(umm AllocationSize, memory_arena *BlockMemory, b32 Multithreaded = False);

link_internal b32
DeinitGpuHeap(gpu_heap_allocator *Heap);

link_internal b32
GpuHeapMap(gpu_heap_allocator *Heap, GLenum Access = GL_MAP_WRITE_BIT);

link_internal b32
GpuHeapUnmap(gpu_heap_allocator *Heap);

link_internal gpu_heap_allocation
GpuHeapAllocate(gpu_heap_allocator *Heap, umm RequestedSize);

link_internal void
GpuHeapDeallocate(gpu_heap_allocator *Heap, gpu_heap_allocation *Allocation);

link_internal b32
IsGpuHeapAllocated(gpu_heap_allocator *Heap, void *Allocation);
