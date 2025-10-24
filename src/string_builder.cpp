link_internal string_builder
StringBuilder()
{
  string_builder Result = {};
  Result.Chunks.Memory = AllocateArena();
  return Result;
}

link_internal void
Discard(string_builder* Builder)
{
  VaporizeArena(Builder->Chunks.Memory);
}

link_internal counted_string
Finalize(string_builder *Builder, memory_arena* PermMemory, b32 IncludeNullTerminator = False)
{
  TIMED_FUNCTION();
  u32 TotalMemRequired = 0;

  if (IncludeNullTerminator)
  {
    TotalMemRequired += 1;
  }

  RangeIterator_t(u32, ElementIndex, Builder->Chunks.ElementCount)
  {
    counted_string* At = GetPtr(&Builder->Chunks, ElementIndex);
    TotalMemRequired += At->Count;
  }

  counted_string Result = CountedString(TotalMemRequired, PermMemory);

  u32 AtIndex = 0;

  RangeIterator_t(u32, ElementIndex, Builder->Chunks.ElementCount)
  {
    counted_string* At = GetPtr(&Builder->Chunks, ElementIndex);

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

  Discard(Builder);

  return Result;
}

link_internal void
Append(string_builder* Builder, counted_string String)
{
  Push(&Builder->Chunks, &String);
}

link_internal void
Prepend(string_builder* Builder, counted_string String)
{
  Shift(&Builder->Chunks, &String);
}

