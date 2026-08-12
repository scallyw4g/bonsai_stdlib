link_internal void
VerifyHeapIntegrity(heap_allocator *Heap);

link_internal void
VerifyHeapAllocationBlock(heap_allocator *Heap, heap_allocation_block *Block);

link_internal void
VerifyHeapAllocationBlock_(heap_allocator *Heap, heap_allocation_block *Block);

heap_allocator
InitHeap(umm AllocationSize, b32 Multithreaded = False)
{
  heap_allocator Result = {};

  Result.FirstBlock = (heap_allocation_block*)PlatformAllocateSize(AllocationSize);
  Assert(Result.FirstBlock);

  Result.FirstBlock->Size = AllocationSize - sizeof(heap_allocation_block);
  Result.FirstBlock->Type = AllocationType_Free;
  Result.FirstBlock->Magic0 = HEAP_MAGIC_NUMBER;
  Result.FirstBlock->Magic1 = HEAP_MAGIC_NUMBER;

  Result.Size = AllocationSize;

  heap_allocation_block* EndBlock = (heap_allocation_block*)((u8*)Result.FirstBlock + (AllocationSize - sizeof(heap_allocation_block)));
  EndBlock->Type = AllocationType_End;
  EndBlock->Size = 0;
  EndBlock->PrevAllocationSize = AllocationSize - sizeof(heap_allocation_block);
  EndBlock->Magic0 = HEAP_MAGIC_NUMBER;
  EndBlock->Magic1 = HEAP_MAGIC_NUMBER;

  return Result;
}

link_internal b32
DeinitHeap(heap_allocator *Heap)
{
  b32 Result = PlatformDeallocate(Cast(u8*, Heap->FirstBlock), Heap->Size);
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
GetPrevBlock(heap_allocator *Allocator, heap_allocation_block* Current)
{
  heap_allocation_block* Result = 0;

  if (Current->PrevAllocationSize)
      Result = (heap_allocation_block*)((u8*)Current - Current->PrevAllocationSize);

  if (Result) VerifyHeapAllocationBlock_(Allocator, Result);
  return Result;
}

heap_allocation_block*
GetNextBlock(heap_allocator *Allocator, heap_allocation_block* Current)
{
  Assert(Current->Magic0 == HEAP_MAGIC_NUMBER);
  Assert(Current->Magic1 == HEAP_MAGIC_NUMBER);

  heap_allocation_block* Result = 0;

  if (Current->Size)
  {
    u8 *EndOfHeap = Cast(u8*, Allocator->FirstBlock) + Allocator->Size;
    u8 *Next      = Cast(u8*, Current) + Current->Size;

    if (Next < EndOfHeap)
    {
      Result = Cast(heap_allocation_block*, Next);

      if (Result->Type != AllocationType_End)
      {
        Assert(Result->Type < AllocationType_Error);
        Assert(Result->Magic0 == HEAP_MAGIC_NUMBER);
        Assert(Result->Magic1 == HEAP_MAGIC_NUMBER);
      }
      else
      {
        Result = 0;
      }
    }
    else
    {
      Assert(Next == EndOfHeap);
    }
  }

  if (Result) VerifyHeapAllocationBlock_(Allocator, Result);
  return Result;
}

void
CondenseAllocations(heap_allocator *Heap, heap_allocation_block* B1, heap_allocation_block* B2)
{
  Assert(B1->Type == AllocationType_Free || B2->Type == AllocationType_Free);

  B1->Type = AllocationType_Free;
  B2->Type = AllocationType_Free;

  heap_allocation_block* First = B1 < B2 ? B1 : B2;
  heap_allocation_block* Second =  B1 < B2 ? B2 : B1;

  First->Size += Second->Size;
  VerifyHeapAllocationBlock(Heap, First);

  heap_allocation_block* BlockAfterSecond = GetNextBlock(Heap, Second);
  if (BlockAfterSecond)
  {
    BlockAfterSecond->PrevAllocationSize = First->Size;
    VerifyHeapAllocationBlock(Heap, BlockAfterSecond);
  }

  *Second = {};
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
  AcquireFutex(&Allocator->Futex);
  VerifyHeapIntegrity(Allocator);

  Assert(Allocator->FirstBlock && Allocator->Size);

  u8 *Result = 0;
  umm EndOfHeap = umm(Allocator->FirstBlock) + Allocator->Size;

  umm AllocationSize = RequestedSize + sizeof(heap_allocation_block);
  umm PrevAllocationSize = 0;

  heap_allocation_block *AtBlock = Allocator->FirstBlock;
  while ( AtBlock && umm(AtBlock) < EndOfHeap )
  {
    if (AtBlock->Size >= AllocationSize &&
        AtBlock->Type == AllocationType_Free)
    {
      Result = (u8*)( umm(AtBlock) + sizeof(heap_allocation_block) );

      umm AtBlockSize = AtBlock->Size;

      AtBlock->Type = AllocationType_Reserved;
      AtBlock->PrevAllocationSize = PrevAllocationSize;

      // If AtBlock has enough room to split (at least 32 bytes + header), split
      if (AtBlockSize > AllocationSize + sizeof(heap_allocation_block) + 32)
      {
        AtBlock->Size = AllocationSize;

        heap_allocation_block *NextAt = (heap_allocation_block*)( umm(AtBlock) + AtBlock->Size );
        NextAt->Size = AtBlockSize - AllocationSize;
        NextAt->Type = AllocationType_Free;
        NextAt->PrevAllocationSize = AllocationSize;
        NextAt->Magic0 = HEAP_MAGIC_NUMBER;
        NextAt->Magic1 = HEAP_MAGIC_NUMBER;

        Assert(GetNextBlock(Allocator, AtBlock) == NextAt);
        VerifyHeapAllocationBlock(Allocator, NextAt);
      }
      else
      {
        // The AtBlock wasn't worth splitting, we just waste a bit of space at the end.
      }

      break;
    }
    else
    {
      PrevAllocationSize = AtBlock->Size;
      AtBlock = GetNextBlock(Allocator, AtBlock);
    }
  }

  VerifyHeapIntegrity(Allocator);
  ReleaseFutex(&Allocator->Futex);

#endif

  if (Result == 0) { SoftError("Heap Allocation failed."); }
  return Result;
}

link_internal void
HeapDeallocate(heap_allocator *Allocator, void* Allocation)
{
#if 0
  free(Allocation);
#else
  AcquireFutex(&Allocator->Futex);
  VerifyHeapIntegrity(Allocator);

  Assert(Allocation);
  Assert(IsHeapAllocated(Allocator, Allocation));

  heap_allocation_block* AllocationBlock = (heap_allocation_block*)((u8*)Allocation - sizeof(heap_allocation_block));
  heap_allocation_block AllocationBlockData = *AllocationBlock;

  VerifyHeapAllocationBlock(Allocator, AllocationBlock);

  ZeroMemory(Allocation, AllocationBlock->Size-sizeof(heap_allocation_block));
  AllocationBlock->Type = AllocationType_Free;

  VerifyHeapIntegrity(Allocator);

  heap_allocation_block* Next = GetNextBlock(Allocator, AllocationBlock);
  heap_allocation_block* Prev = GetPrevBlock(Allocator, AllocationBlock);

  heap_allocation_block NextBlockData = {};
  heap_allocation_block PrevBlockData = {};

  if (Next) NextBlockData = *Next;
  if (Prev) PrevBlockData = *Prev;

  if (Next && Next->Type == AllocationType_Free)
  {
    CondenseAllocations(Allocator, AllocationBlock, Next);
    VerifyHeapIntegrity(Allocator);
  }

  if (Prev && Prev->Type == AllocationType_Free)
  {
    CondenseAllocations(Allocator, AllocationBlock, Prev);
    VerifyHeapIntegrity(Allocator);
  }


  VerifyHeapIntegrity(Allocator);
  ReleaseFutex(&Allocator->Futex);
#endif

  return;
}


link_internal b32
IsHeapAllocated(heap_allocator *Heap, void *Allocation)
{
  u8 *Start = Cast(u8*, Heap->FirstBlock);
  u8 *End   = Cast(u8*, Heap->FirstBlock)+Heap->Size;
  b32 Result = Allocation > Start && Allocation < End;
  return Result;
}

link_internal b32
MaybeDeallocate(heap_allocator *Heap, void *Allocation)
{
  b32 Result = IsHeapAllocated(Heap, Allocation);
  if (Result) HeapDeallocate(Heap, Allocation);
  return Result;
}

link_internal void
VerifyHeapAllocationBlock_(heap_allocator *Heap, heap_allocation_block *Block)
{
  u8 *StartOfHeap = Cast(u8*, Heap->FirstBlock);
  u8 *EndOfHeap = Cast(u8*, Heap->FirstBlock) + Heap->Size;

  u8 *u8Block = Cast(u8*, Block);
  Assert(u8Block >= StartOfHeap);
  Assert(u8Block             < EndOfHeap);
  Assert(u8Block+Block->Size < EndOfHeap);

  Assert(Block->Magic0 == HEAP_MAGIC_NUMBER);
  Assert(Block->Magic1 == HEAP_MAGIC_NUMBER);

  Assert(Block->Type == AllocationType_Free || Block->Type == AllocationType_Reserved);

  Assert(Block->Size);
  Assert(Block->Size               < Heap->Size);
  Assert(Block->PrevAllocationSize < Heap->Size);

}

link_internal void
VerifyHeapAllocationBlock(heap_allocator *Heap, heap_allocation_block *Block)
{
  auto Next = GetNextBlock(Heap, Block);
  auto Prev = GetPrevBlock(Heap, Block);

  VerifyHeapAllocationBlock_(Heap, Block);
  if (Next) VerifyHeapAllocationBlock_(Heap, Next);
  if (Prev) VerifyHeapAllocationBlock_(Heap, Prev);
}


link_internal void
VerifyHeapIntegrity(heap_allocator *Heap)
{
  u8 *StartOfHeap = Cast(u8*, Heap->FirstBlock);
  u8 *EndOfHeap = Cast(u8*, Heap->FirstBlock) + Heap->Size;


  u8 *At = Cast(u8*, Heap->FirstBlock);

  while ( At && At < EndOfHeap )
  {
    VerifyHeapAllocationBlock(Heap, Cast(heap_allocation_block*, At));

    At = Cast(u8*, GetNextBlock(Heap, Cast(heap_allocation_block*, At)));
  }
}
