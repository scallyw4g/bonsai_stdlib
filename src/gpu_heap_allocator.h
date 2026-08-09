#pragma once

// GPU-backed heap allocator.
// Metadata is stored separately in a block descriptor arena and the allocator
// uses OpenGL buffer storage as the backing memory.
//
// Usage:
//   gpu_heap_allocator Heap = InitGpuHeap(Megabytes(4), BlockMemory);
//   GpuHeapMap(&Heap, GL_MAP_WRITE_BIT);
//   gpu_heap_allocation A = GpuHeapAllocate(&Heap, 256, myData);
//   GpuHeapUnmap(&Heap);
//   DeinitGpuHeap(&Heap);
struct gpu_heap_allocator
{
  gpu_mapped_element_buffer Storage;

  heap_allocation_block *FirstBlock;

  heap_allocation_block *Freelist; // NOTE(Jesse): List of free blocks

  memory_arena *BlockMemory;

  bonsai_futex Futex;
};

struct gpu_heap_allocation
{
  untextured_3d_geometry_buffer Data;
  umm BaseOffsetInElements;
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
GpuHeapDeallocate(gpu_heap_allocator *Heap, void *Allocation);

link_internal b32
IsGpuHeapAllocated(gpu_heap_allocator *Heap, void *Allocation);
