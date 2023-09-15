heap_allocator
InitHeap(umm AllocationSize)
{
  heap_allocator Result = {};

  Result.FirstBlock = (heap_allocation_block*)PlatformAllocateSize(AllocationSize);
  Result.FirstBlock->Size = AllocationSize - sizeof(heap_allocation_block);
  Result.FirstBlock->Type = AllocationType_Free;

  Result.Size = AllocationSize;

  heap_allocation_block* EndBlock = (heap_allocation_block*)((u8*)Result.FirstBlock + (AllocationSize - sizeof(heap_allocation_block)));
  EndBlock->Type = AllocationType_Reserved;
  EndBlock->Size = 0;
  EndBlock->PrevAllocationSize = AllocationSize - sizeof(heap_allocation_block);

  return Result;
}

link_internal umm
OffsetForHeapAllocation(heap_allocator *Allocator, u8 *Alloc)
{
  Assert(Alloc > (u8*)Allocator->FirstBlock);
  Assert(Alloc < (u8*)Allocator->FirstBlock+Allocator->Size);

  umm Result = umm(Alloc - (u8*)Allocator->FirstBlock);
  return Result;
}

link_internal u8*
HeapAllocate(heap_allocator *Allocator, umm RequestedSize)
{

  Assert(Allocator->FirstBlock && Allocator->Size);

#if 0
  u8 *Result = (u8*)calloc(umm(RequestedSize), u64(1));
#else
  /* NotImplemented; */
  u8* Result = 0;
  u8* EndOfHeap = (u8*)Allocator->FirstBlock + Allocator->Size;

  umm AllocationSize = RequestedSize + sizeof(heap_allocation_block);
  umm PrevAllocationSize = 0;

  heap_allocation_block* At = Allocator->FirstBlock;
  while ( (u8*)At < EndOfHeap )
  {
    if (At->Size > AllocationSize && At->Type == AllocationType_Free)
    {
      Result = (u8*)(At + 1);

      umm InitialBlockSize = At->Size;

      At->Size = AllocationSize;
      At->Type = AllocationType_Reserved;
      At->PrevAllocationSize = PrevAllocationSize;

      heap_allocation_block *NextAt = (heap_allocation_block*)((u8*)At + At->Size);
      NextAt->Size = InitialBlockSize - AllocationSize;
      NextAt->Type = AllocationType_Free;
      NextAt->PrevAllocationSize = AllocationSize;

      break;
    }
    else
    {
      PrevAllocationSize = At->Size;
      At = (heap_allocation_block*)((u8*)At + At->Size);

      if (At->Size == 0)
      {
        Error("Ran out of heap memory, panic!");
        break;
      }
    }
  }
#endif

  return Result;
}

heap_allocation_block*
GetPrevBlock(heap_allocation_block* Current)
{
  heap_allocation_block* Result = 0;

  if (Current->PrevAllocationSize)
      Result = (heap_allocation_block*)((u8*)Current + Current->PrevAllocationSize);

  return Result;
}

heap_allocation_block*
GetNextBlock(heap_allocation_block* Current)
{
  heap_allocation_block* Result = 0;

  if (Current->Size)
      Result = (heap_allocation_block*)((u8*)Current + Current->Size);

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

void
HeapDeallocate(void* Allocation)
{
  Assert(Allocation);

  heap_allocation_block* AllocationBlock = (heap_allocation_block*)((u8*)Allocation - sizeof(heap_allocation_block));

  heap_allocation_block* Next = GetNextBlock(AllocationBlock);
  if (Next && Next->Type == AllocationType_Free)
  {
    CondenseAllocations(AllocationBlock, Next);
    return;
  }

  heap_allocation_block* Prev = GetPrevBlock(AllocationBlock);
  if (Prev && Prev->Type == AllocationType_Free)
  {
    CondenseAllocations(AllocationBlock, Prev);
    return;
  }

  AllocationBlock->Type = AllocationType_Free;

  return;
}



























