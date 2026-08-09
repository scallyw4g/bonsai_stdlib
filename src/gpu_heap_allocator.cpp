
link_internal heap_allocation_block *
AllocateHeapAllocationBlock(gpu_heap_allocator *Heap)
{
  Assert(Heap->BlockMemory);
  heap_allocation_block *Result = Cast(heap_allocation_block*, PushStruct(Heap->BlockMemory, sizeof(*Result), alignof(heap_allocation_block), False));
  Assert(Result);
  *Result = {};
  Result->Magic0 = HEAP_MAGIC_NUMBER;
  Result->Magic1 = HEAP_MAGIC_NUMBER;
  return Result;
}

link_internal void
LinkFreeBlock(gpu_heap_allocator *Heap, heap_allocation_block *Block)
{
  Assert(Block->Type == AllocationType_Free);
  Block->NextFree = Heap->Freelist;
  Heap->Freelist = Block;
}

link_internal void
UnlinkFreeBlock(gpu_heap_allocator *Heap, heap_allocation_block *Block)
{
  if (Heap->Freelist == Block)
  {
    Heap->Freelist = Block->NextFree;
    Block->NextFree = 0;
    return;
  }

  heap_allocation_block *At = Heap->Freelist;
  while (At && At->NextFree != Block)
  {
    At = At->NextFree;
  }

  Assert(At);
  At->NextFree = Block->NextFree;
  Block->NextFree = 0;
}

link_internal heap_allocation_block *
FindPreviousBlock(gpu_heap_allocator *Heap, heap_allocation_block *Current)
{
  heap_allocation_block *Result = 0;
  heap_allocation_block *At = Heap->FirstBlock;
  while (At && At->Next != Current)
  {
    At = At->Next;
  }
  if (At && At->Next == Current)
  {
    Result = At;
  }
  return Result;
}

link_internal heap_allocation_block *
FindBlockByOffset(gpu_heap_allocator *Heap, umm BaseOffset)
{
  heap_allocation_block *At = Heap->FirstBlock;
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

  Result.BlockMemory = BlockMemory;
  Result.FirstBlock = 0;
  Result.Freelist = 0;

  u32 ElementCount = Cast(u32, RequestedHeapSizeInBytes / sizeof(v3_u8));
  AllocateGpuBuffer(&Result.Storage, DataType_v3_u8, ElementCount);

  umm HeapSize = Cast(umm, ElementCount) * sizeof(v3_u8);

  // Replace backing store with persistent buffer storage and map persistently
  auto GL = GetGL();
  // Vertex buffer
  GL->BindBuffer(GL_ARRAY_BUFFER, Result.Storage.Handles.Handles[mesh_VertexHandle]);
  GL->BufferStorage(GL_ARRAY_BUFFER, Cast(GLsizeiptr, HeapSize), 0, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
  // Normal buffer
  GL->BindBuffer(GL_ARRAY_BUFFER, Result.Storage.Handles.Handles[mesh_NormalHandle]);
  GL->BufferStorage(GL_ARRAY_BUFFER, Cast(GLsizeiptr, HeapSize), 0, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
  // Material buffer
  u32 MaterialBufferSize = Cast(u32, ElementCount) * Cast(u32, sizeof(matl));
  GL->BindBuffer(GL_ARRAY_BUFFER, Result.Storage.Handles.Handles[mesh_MatHandle]);
  GL->BufferStorage(GL_ARRAY_BUFFER, Cast(GLsizeiptr, MaterialBufferSize), 0, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

  // Map entire buffers once
  GL->BindBuffer(GL_ARRAY_BUFFER, Result.Storage.Handles.Handles[mesh_VertexHandle]);
  Result.Storage.Buffer.Verts = Cast(v3_u8*, GL->MapBufferRange(GL_ARRAY_BUFFER, 0, Cast(GLsizeiptr, HeapSize), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
  GL->BindBuffer(GL_ARRAY_BUFFER, Result.Storage.Handles.Handles[mesh_NormalHandle]);
  Result.Storage.Buffer.Normals = Cast(v3_u8*, GL->MapBufferRange(GL_ARRAY_BUFFER, 0, Cast(GLsizeiptr, HeapSize), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
  GL->BindBuffer(GL_ARRAY_BUFFER, Result.Storage.Handles.Handles[mesh_MatHandle]);
  Result.Storage.Buffer.Mat = Cast(matl*, GL->MapBufferRange(GL_ARRAY_BUFFER, 0, Cast(GLsizeiptr, MaterialBufferSize), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
  GL->BindBuffer(GL_ARRAY_BUFFER, 0);

  AssertNoGlErrors;

  Result.Storage.Handles.Mapped = True;

  heap_allocation_block *InitialBlock = AllocateHeapAllocationBlock(&Result);
  InitialBlock->Type = AllocationType_Free;
  InitialBlock->Size = HeapSize;
  InitialBlock->BaseOffset = 0;
  InitialBlock->Next = 0;
  InitialBlock->NextFree = 0;

  Result.FirstBlock = InitialBlock;
  Result.Freelist = InitialBlock;

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
  Heap->Freelist = 0;
  Heap->BlockMemory = 0;

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
  Assert(Heap->Storage.Handles.Mapped);
  gpu_heap_allocation Allocation = {};

  if (!Heap->Freelist)
  {
    SoftError("GpuHeap allocation failed: no free blocks.");
    return Allocation;
  }

  AcquireFutex(&Heap->Futex);
  heap_allocation_block *At = Heap->Freelist;
  while (At && At->Size < ElementCount)
  {
    At = At->NextFree;
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

  if (At->Size > ElementCount)
  {
    heap_allocation_block *Remainder = AllocateHeapAllocationBlock(Heap);
    Remainder->Type = AllocationType_Free;
    Remainder->Size = At->Size - ElementCount;
    Remainder->BaseOffset = At->BaseOffset + ElementCount;
    /* Remainder->Next = At->Next; */
    Remainder->NextFree = 0;

    At->Size = ElementCount;
    At->Next = Remainder;
    LinkFreeBlock(Heap, Remainder);
  }

  At->Type = AllocationType_Reserved;

  ReleaseFutex(&Heap->Futex);

  Allocation.BaseOffsetInElements = At->BaseOffset;
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
GpuHeapDeallocate(gpu_heap_allocator *Heap, void *Allocation)
{
  Assert(Heap->Storage.Handles.Mapped);
  AcquireFutex(&Heap->Futex);
  Assert(Allocation);

  u8 *Base = Cast(u8*, Heap->Storage.Buffer.Verts);
  Assert(Base);
  Assert(Allocation >= Base);

  umm BaseOffset = Cast(umm, Cast(u8*, Allocation) - Base);
  heap_allocation_block *Block = FindBlockByOffset(Heap, BaseOffset);
  Assert(Block);
  Assert(Block->Type == AllocationType_Reserved);

  Block->Type = AllocationType_Free;
  Block->NextFree = 0;

  heap_allocation_block *Next = Block->Next;
  if (Next && Next->Type == AllocationType_Free)
  {
    UnlinkFreeBlock(Heap, Next);
    Block->Size += Next->Size;
    Block->Next = Next->Next;
  }

  heap_allocation_block *Prev = FindPreviousBlock(Heap, Block);
  if (Prev && Prev->Type == AllocationType_Free)
  {
    UnlinkFreeBlock(Heap, Prev);
    Prev->Size += Block->Size;
    Prev->Next = Block->Next;
    Block = Prev;
  }

  LinkFreeBlock(Heap, Block);
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
  heap_allocation_block *Block = FindBlockByOffset(Heap, BaseOffset);
  return Block && Block->Type == AllocationType_Reserved;
}
