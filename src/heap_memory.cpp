heap_allocator
InitHeap(umm AllocationSize, b32 Multithreaded = False)
{
  heap_allocator Result = {};

  if (Multithreaded) { InitializeFutex(&Result.Futex); }
  else { Result.OwnedByThread = ThreadLocal_ThreadIndex; }

  Result.FirstBlock = (heap_allocation_block*)PlatformAllocateSize(AllocationSize);
  Assert(Result.FirstBlock);

  Result.FirstBlock->Size = AllocationSize - sizeof(heap_allocation_block);
  Result.FirstBlock->Type = AllocationType_Free;
  Result.FirstBlock->Magic0 = HEAP_MAGIC_NUMBER;
  Result.FirstBlock->Magic1 = HEAP_MAGIC_NUMBER;

  Result.Size = AllocationSize;

  heap_allocation_block* EndBlock = (heap_allocation_block*)((u8*)Result.FirstBlock + (AllocationSize - sizeof(heap_allocation_block)));
  EndBlock->Type = AllocationType_Reserved;
  EndBlock->Size = 0;
  EndBlock->PrevAllocationSize = AllocationSize - sizeof(heap_allocation_block);
  EndBlock->Magic0 = HEAP_MAGIC_NUMBER;
  EndBlock->Magic1 = HEAP_MAGIC_NUMBER;

  return Result;
}

link_internal umm
OffsetForHeapAllocation(heap_allocator *Allocator, u8 *Alloc)
{
  Assert(Alloc > (u8*)Allocator->FirstBlock);
  Assert(Alloc < (u8*)Allocator->FirstBlock+Allocator->Size);

  umm Result = umm(Alloc - Cast(u8*, Allocator->FirstBlock));
  Assert(Result >= sizeof(heap_allocation_block));

  return Result;
}

heap_allocation_block*
GetPrevBlock(heap_allocation_block* Current)
{
  heap_allocation_block* Result = 0;

  if (Current->PrevAllocationSize)
      Result = (heap_allocation_block*)((u8*)Current - Current->PrevAllocationSize);

  return Result;
}

heap_allocation_block*
GetNextBlock(heap_allocation_block* Current)
{
  Assert(Current->Magic0 == HEAP_MAGIC_NUMBER);
  Assert(Current->Magic1 == HEAP_MAGIC_NUMBER);

  heap_allocation_block* Result = 0;

  if (Current->Size)
  {
    Result = (heap_allocation_block*)( umm(Current) + Current->Size );
    Assert(Result->Type < AllocationType_Error);

    Assert(Result->Magic0 == HEAP_MAGIC_NUMBER);
    Assert(Result->Magic1 == HEAP_MAGIC_NUMBER);
  }

  return Result;
}

void
CondenseAllocations(heap_allocation_block* B1, heap_allocation_block* B2)
{
  Assert(B1->Type == AllocationType_Free || B2->Type == AllocationType_Free);

  B1->Type = AllocationType_Free;
  B2->Type = AllocationType_Free;

  heap_allocation_block* First = B1 < B2 ? B1 : B2;
  heap_allocation_block* Second =  B1 < B2 ? B2 : B1;

  First->Size += Second->Size;

  heap_allocation_block* BlockAfterSecond = GetNextBlock(Second);
  if (BlockAfterSecond)
  {
    BlockAfterSecond->PrevAllocationSize = First->Size;
  }

  return;
}

link_internal u8*
GetDataPointer(heap_allocation_block* Block)
{
  u8 *Result = (u8*)Block + sizeof(heap_allocation_block);
  return Result;
}

link_internal umm
GetDataSize(heap_allocation_block* Block)
{
  umm Result = Block->Size - sizeof(heap_allocation_block);
  return Result;
}





// Returns pointer to valid bytes to write to.
link_internal u8*
HeapAllocate(heap_allocator *Allocator, umm RequestedSize)
{
#if 0
  u8* Result = (u8*)calloc(1, RequestedSize);
#else
  if (Allocator->Futex.Initialized == True)
  {
    AcquireFutex(&Allocator->Futex);
  }
  else
  {
    Assert(Allocator->OwnedByThread == ThreadLocal_ThreadIndex);
  }

  Assert(Allocator->FirstBlock && Allocator->Size);

  u8 *Result = 0;
  umm EndOfHeap = umm(Allocator->FirstBlock) + Allocator->Size;

  umm AllocationSize = RequestedSize + sizeof(heap_allocation_block);
  umm PrevAllocationSize = 0;

  heap_allocation_block *AtBlock = Allocator->FirstBlock;
  while ( umm(AtBlock) < EndOfHeap )
  {
    // TODO(Jesse): Should this not be >= .. ?
    if (AtBlock->Size > AllocationSize && AtBlock->Type == AllocationType_Free)
    {
      Result = (u8*)( umm(AtBlock) + sizeof(heap_allocation_block) );

      umm InitialBlockSize = AtBlock->Size;

      AtBlock->Size = AllocationSize;
      AtBlock->Type = AllocationType_Reserved;
      AtBlock->PrevAllocationSize = PrevAllocationSize;

      heap_allocation_block *NextAt = (heap_allocation_block*)( umm(AtBlock) + AtBlock->Size );
      NextAt->Size = InitialBlockSize - AllocationSize;
      NextAt->Type = AllocationType_Free;
      NextAt->PrevAllocationSize = AllocationSize;
      NextAt->Magic0 = HEAP_MAGIC_NUMBER;
      NextAt->Magic1 = HEAP_MAGIC_NUMBER;

      Assert(GetNextBlock(AtBlock) == NextAt);

      break;
    }
    else
    {
      PrevAllocationSize = AtBlock->Size;
      AtBlock = GetNextBlock(AtBlock);

      if (AtBlock->Size == 0)
      {
        SoftError("Heap allocation failed.");
        break;
      }
    }
  }

  if (Allocator->Futex.Initialized == True)
  {
    ReleaseFutex(&Allocator->Futex);
  }

#endif
  return Result;
}

void
HeapDeallocate(heap_allocator *Allocator, void* Allocation)
{
#if 0
  free(Allocation);
#else
  if (Allocator->Futex.Initialized == True)
  {
    AcquireFutex(&Allocator->Futex);
  }
  else
  {
    Assert(Allocator->OwnedByThread == ThreadLocal_ThreadIndex);
  }

  Assert(Allocation);

  heap_allocation_block* AllocationBlock = (heap_allocation_block*)((u8*)Allocation - sizeof(heap_allocation_block));

  heap_allocation_block* Next = GetNextBlock(AllocationBlock);
  if (Next && Next->Type == AllocationType_Free)
  {
    CondenseAllocations(AllocationBlock, Next);
  }

  heap_allocation_block* Prev = GetPrevBlock(AllocationBlock);
  if (Prev && Prev->Type == AllocationType_Free)
  {
    CondenseAllocations(AllocationBlock, Prev);
  }

  AllocationBlock->Type = AllocationType_Free;

  if (Allocator->Futex.Initialized == True) { ReleaseFutex(&Allocator->Futex); }
#endif

  return;
}

