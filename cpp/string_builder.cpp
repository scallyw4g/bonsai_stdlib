link_internal counted_string
Finalize(string_builder* Builder, memory_arena* PermMemory, b32 IncludeNullTerminator = False)
{
  TIMED_FUNCTION();
  u32 TotalMemRequired = 0;

  if (IncludeNullTerminator)
  {
    TotalMemRequired += 1;
  }

  ITERATE_OVER(&Builder->Chunks)
  {
    counted_string* At = GET_ELEMENT(Iter);
    TotalMemRequired += At->Count;
  }

  counted_string Result = CountedString(TotalMemRequired, PermMemory);

  u32 AtIndex = 0;

  ITERATE_OVER(&Builder->Chunks)
  {
    counted_string* At = GET_ELEMENT(Iter);
    MemCopy((u8*)At->Start, (u8*)(Result.Start+AtIndex), At->Count);
    AtIndex += At->Count;
    Assert(AtIndex <= Result.Count);
  }

  if (IncludeNullTerminator)
  {
    Assert(AtIndex+1 == Result.Count);
    Assert(Result.Start[AtIndex] == 0);
  }
  else
  {
    Assert(AtIndex == Result.Count);
  }

  VaporizeArena(Builder->Memory);

  return Result;
}


