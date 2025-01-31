struct string_builder
{
  counted_string_stream Chunks = { AllocateArena(0, True, False), 0, 0, 0 };
};

link_internal string_builder
StringBuilder(memory_arena *Memory)
{
  string_builder Result = {{0,0,0,0}};
  Assert(Result.Chunks.Memory == 0);
  Result.Chunks.Memory = Memory;
  return Result;
}

link_internal void
Append(string_builder* Builder, counted_string String)
{
  Push(&Builder->Chunks, String);
}


