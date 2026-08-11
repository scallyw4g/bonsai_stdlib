
link_internal gpu_heap_allocation *
AllocateGpuHeapAllocation(gpu_heap_allocator *Heap)
{
  Assert(Heap->gpu_heap_allocation_Memory);

  gpu_heap_allocation *Result = Heap->gpu_heap_allocation_Freelist;
  if (Result)
  {
    Heap->gpu_heap_allocation_Freelist = Result->Next;
  }
  else
  {
    Result = Cast(gpu_heap_allocation*, PushStruct(Heap->gpu_heap_allocation_Memory, sizeof(*Result), alignof(gpu_heap_allocation), False));
  }

  Assert(Result);
  *Result = {};
  return Result;
}

link_internal void
FreeGpuHeapAllocation(gpu_heap_allocator *Heap, gpu_heap_allocation *Block)
{
  Block->Prev = 0;
  Block->Next = Heap->gpu_heap_allocation_Freelist;
  Heap->gpu_heap_allocation_Freelist = Block;
}

link_internal gpu_heap_allocation *
FindBlockByOffset(gpu_heap_allocator *Heap, umm BaseOffset)
{
  gpu_heap_allocation *At = Heap->FirstBlock;
  while (At)
  {
    if (At->BaseOffset == BaseOffset)
    {
      return At;
    }
    At = At->Next;
  }
  return 0;
}

link_internal umm
GpuHeapCapacityInElements(gpu_heap_allocator *Heap)
{
  Assert(Heap->Storage.Handles.ElementType == DataType_v3_u8);
  return Cast(umm, Heap->Storage.Handles.ElementCount);
}

link_internal umm
GpuHeapCapacityBytes(gpu_heap_allocator *Heap)
{
  Assert(Heap->Storage.Handles.ElementType == DataType_v3_u8);
  return Cast(umm, Heap->Storage.Handles.ElementCount) * sizeof(v3_u8);
}


link_internal gpu_heap_allocator
InitGpuHeap(umm RequestedHeapSizeInBytes, memory_arena *BlockMemory, b32 Multithreaded)
{
  gpu_heap_allocator Result = {};
  Assert(BlockMemory);

  Result.gpu_heap_allocation_Memory = BlockMemory;
  Result.FirstBlock = 0;
  Result.gpu_heap_allocation_Freelist = 0;

  u32 ElementCount = Cast(u32, RequestedHeapSizeInBytes / sizeof(v3_u8));
  AllocateGpuBuffer(&Result.Storage, DataType_v3_u8, ElementCount);

  umm HeapSize = Cast(umm, ElementCount) * sizeof(v3_u8);

  gpu_heap_allocation *InitialBlock = AllocateGpuHeapAllocation(&Result);
  InitialBlock->Type = AllocationType_Free;
  InitialBlock->SizeInElements = HeapSize;
  InitialBlock->BaseOffset = 0;
  InitialBlock->BaseOffsetInElements = 0;
  InitialBlock->Prev = 0;
  InitialBlock->Next = 0;

  Result.FirstBlock = InitialBlock;

  return Result;
}

link_internal b32
DeinitGpuHeap(gpu_heap_allocator *Heap)
{
  b32 Result = True;
  if (Heap->Storage.Handles.Mapped)
  {
    Result &= GpuHeapUnmap(Heap);
  }

  DeallocateGpuBuffer(&Heap->Storage);

  Heap->FirstBlock = 0;
  Heap->gpu_heap_allocation_Freelist = 0;
  Heap->gpu_heap_allocation_Memory = 0;

  return Result;
}

link_internal b32
GpuHeapMap(gpu_heap_allocator *Heap, GLenum Access)
{
  // Already mapped persistently at InitGpuHeap
  return Heap->Storage.Handles.Mapped ? True : False;
}

link_internal b32
GpuHeapUnmap(gpu_heap_allocator *Heap)
{
  if (!Heap->Storage.Handles.Mapped)
  {
    return True;
  }

  b32 Result = UnmapGpuBuffer(&Heap->Storage.Handles);
  return Result;
}

link_internal gpu_heap_allocation
GpuHeapAllocate(gpu_heap_allocator *Heap, umm ElementCount)
{
  /* Assert(Heap->Storage.Handles.Mapped); */
  gpu_heap_allocation Allocation = {};

  AcquireFutex(&Heap->Futex);
  gpu_heap_allocation *At = Heap->FirstBlock;
  while (At && (At->Type != AllocationType_Free || At->SizeInElements < ElementCount))
  {
    At = At->Next;
  }

  if (!At)
  {
    SoftError("GpuHeap allocation failed: not enough free space.");
    ReleaseFutex(&Heap->Futex);
    return Allocation;
  }

  Assert(At->Type == AllocationType_Free);

  umm HeapElements = GpuHeapCapacityInElements(Heap);
  Assert(At->BaseOffset + ElementCount <= HeapElements);

  if (At->SizeInElements > ElementCount)
  {
    gpu_heap_allocation *Remainder = AllocateGpuHeapAllocation(Heap);
    Remainder->Type = AllocationType_Free;
    Remainder->SizeInElements = At->SizeInElements - ElementCount;
    Remainder->BaseOffset = At->BaseOffset + ElementCount;
    Remainder->BaseOffsetInElements = Remainder->BaseOffset;
    Remainder->Prev = At;
    Remainder->Next = At->Next;
    if (Remainder->Next)
    {
      Remainder->Next->Prev = Remainder;
    }

    At->SizeInElements = ElementCount;
    At->Next = Remainder;
  }

  At->Type = AllocationType_Reserved;
  At->BaseOffsetInElements = At->BaseOffset;

  ReleaseFutex(&Heap->Futex);

  Allocation = *At;
  Allocation.Prev = 0;
  Allocation.Next = 0;
  Allocation.Data.Type = DataType_v3_u8;
  Allocation.Data.Next = 0;

  u8 *BaseVerts   = Cast(u8*, Heap->Storage.Buffer.Verts);
  u8 *BaseNormals = Cast(u8*, Heap->Storage.Buffer.Normals);
  u8 *BaseMat     = Cast(u8*, Heap->Storage.Buffer.Mat);

  Allocation.Data.Verts   = Cast(v3_u8*, BaseVerts + At->BaseOffset);
  Allocation.Data.Normals = Cast(v3_u8*, BaseNormals + At->BaseOffset);
  Allocation.Data.Mat     = Cast(matl*, BaseMat + At->BaseOffset);

  Allocation.Data.End = Cast(u32, ElementCount);
  Allocation.Data.At = 0;
  Allocation.Data.Parent = 0;
  Allocation.Data.BufferNeedsToGrow = 0;
  Allocation.Data.Timestamp = 0;

  return Allocation;
}

link_internal void
GpuHeapDeallocate(gpu_heap_allocator *Heap, gpu_heap_allocation *Allocation)
{
  Assert(Allocation);

  AcquireFutex(&Heap->Futex);

  gpu_heap_allocation *Block = FindBlockByOffset(Heap, Allocation->BaseOffsetInElements);
  Assert(Block);
  Assert(Block->Type == AllocationType_Reserved);

  Block->Type = AllocationType_Free;

  gpu_heap_allocation *Next = Block->Next;
  if (Next && Next->Type == AllocationType_Free)
  {
    Block->SizeInElements += Next->SizeInElements;
    Block->Next = Next->Next;
    if (Block->Next)
    {
      Block->Next->Prev = Block;
    }
    FreeGpuHeapAllocation(Heap, Next);
  }

  gpu_heap_allocation *Prev = Block->Prev;
  if (Prev && Prev->Type == AllocationType_Free)
  {
    Prev->SizeInElements += Block->SizeInElements;
    Prev->Next = Block->Next;
    if (Prev->Next)
    {
      Prev->Next->Prev = Prev;
    }
    FreeGpuHeapAllocation(Heap, Block);
  }

  ReleaseFutex(&Heap->Futex);
}

link_internal b32
IsGpuHeapAllocated(gpu_heap_allocator *Heap, void *Allocation)
{
  if (!Heap->Storage.Handles.Mapped || !Heap->Storage.Buffer.Verts)
  {
    return False;
  }

  u8 *Start = Cast(u8*, Heap->Storage.Buffer.Verts);
  u8 *End = Start + GpuHeapCapacityBytes(Heap);
  if (Allocation < Start || Allocation >= End)
  {
    return False;
  }

  umm BaseOffset = Cast(umm, Cast(u8*, Allocation) - Start);
  gpu_heap_allocation *Block = FindBlockByOffset(Heap, BaseOffset);
  return Block && Block->Type == AllocationType_Reserved;
}
